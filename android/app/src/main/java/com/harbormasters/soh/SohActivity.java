package com.harbormasters.soh;

import android.content.Intent;
import android.database.Cursor;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.zip.CRC32;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;


import org.libsdl.app.SDLActivity;

public class SohActivity extends SDLActivity {
    private static final String TAG = "SoH";
    private static final String STAMP = ".unpacked";

    private static final String[] SHIPPED = {
        "soh.o2r",
        "assets",
        "gamecontrollerdb.txt",
    };
    private static final String INTERNAL_ROOT = "assets";
    private static final int REQUEST_PICK_ROM = 1;
    private static final String IMPORT_DIR = "import";
    private static final String FALLBACK_IMPORT_NAME = "rom.z64";

    private volatile boolean romPickPending = false;

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        try {
            unpackAssets(dataDir());
        } catch (IOException e) {
            Log.e(TAG, "Could not unpack the shipped assets", e);
        }
        super.onCreate(savedInstanceState);
        mLayout.post(this::goImmersive);
        mLayout.setOnApplyWindowInsetsListener((view, insets) -> {
            reportInsets(view, insets);
            return view.onApplyWindowInsets(insets);
        });
        mLayout.requestApplyInsets();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            goImmersive();
        }
    }

    @Override
    protected void onDestroy() {
        if (romPickPending) {
            Log.i(TAG, "Releasing the pending ROM pick for shutdown");
            deliverPickedRom(null);
        }
        boolean relaunch = isChangingConfigurations();
        super.onDestroy();
        if (relaunch) {
            Log.i(TAG, "Configuration change needs a new activity; starting a new process");
            startActivity(new Intent(this, SohActivity.class));
        }
        System.exit(0);
    }

    public void openFilePicker() {
        romPickPending = true;
        runOnUiThread(() -> {
            Intent pick = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            pick.addCategory(Intent.CATEGORY_OPENABLE);
            pick.setType("*/*");
            Log.i(TAG, "Opening the system ROM picker");
            try {
                startActivityForResult(pick, REQUEST_PICK_ROM);
            } catch (Exception e) {
                Log.e(TAG, "No document picker available", e);
                deliverPickedRom(null);
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_PICK_ROM) {
            return;
        }
        Uri source = (resultCode == RESULT_OK && data != null) ? data.getData() : null;
        if (source == null) {
            Log.i(TAG, "ROM picker canceled");
            deliverPickedRom(null);
            return;
        }
        new Thread(() -> deliverPickedRom(importPickedRom(source)), "RomImport").start();
    }

    private void deliverPickedRom(String path) {
        romPickPending = false;
        nativeFilePicked(path);
    }

    private String importPickedRom(Uri source) {
        File dir = new File(getCacheDir(), IMPORT_DIR);
        String name = pickedRomName(source);
        File target = new File(dir, name);
        File partial = new File(target.getPath() + ".part");
        try {
            if (!dir.isDirectory() && !dir.mkdirs()) {
                throw new IOException("Could not create " + dir);
            }
            Log.i(TAG, "Importing picked ROM " + source + " as " + name);
            try (InputStream in = getContentResolver().openInputStream(source)) {
                if (in == null) {
                    throw new IOException("Could not open " + source);
                }
                try (OutputStream out = new FileOutputStream(partial)) {
                    byte[] buffer = new byte[256 * 1024];
                    int read;
                    while ((read = in.read(buffer)) != -1) {
                        out.write(buffer, 0, read);
                    }
                }
            }
            if (!partial.renameTo(target)) {
                throw new IOException("Could not move " + partial + " into place");
            }
            Log.i(TAG, "Imported picked ROM to " + target);
            return target.getAbsolutePath();
        } catch (IOException e) {
            Log.e(TAG, "Could not import " + source, e);
            partial.delete();
            return null;
        }
    }

    private String pickedRomName(Uri source) {
        String name = null;
        try (Cursor cursor = getContentResolver().query(source, new String[] { OpenableColumns.DISPLAY_NAME }, null,
                                                         null, null)) {
            if (cursor != null && cursor.moveToFirst() && !cursor.isNull(0)) {
                name = new File(cursor.getString(0)).getName();
            }
        } catch (Exception e) {
            Log.w(TAG, "Could not read the name of " + source, e);
        }
        if (name == null || name.isEmpty() || name.equals(".") || name.equals("..")) {
            return FALLBACK_IMPORT_NAME;
        }
        return name.replaceAll("[^A-Za-z0-9._-]", "_");
    }

    private static native void nativeFilePicked(String path);

    private void goImmersive() {
        Window window = getWindow();
        window.clearFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.setDecorFitsSystemWindows(false);
            WindowInsetsController controller = window.getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            window.getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        }
    }

    private void reportInsets(View view, WindowInsets insets) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            nativeSafeAreaInsets(insets.getSystemWindowInsetLeft(), insets.getSystemWindowInsetTop(),
                                 insets.getSystemWindowInsetRight(), insets.getSystemWindowInsetBottom());
            return;
        }
        android.graphics.Insets reserved =
            insets.getInsets(WindowInsets.Type.displayCutout() | WindowInsets.Type.systemBars());
        nativeSafeAreaInsets(reserved.left, reserved.top, reserved.right, reserved.bottom);

        if (view.getWidth() <= 0 || view.getHeight() <= 0) {
            return;
        }
        android.graphics.Insets gestures = insets.getInsets(WindowInsets.Type.systemGestures());
        List<Rect> exclusions = new ArrayList<>();
        if (gestures.left > 0) {
            exclusions.add(new Rect(0, 0, gestures.left, view.getHeight()));
        }
        if (gestures.right > 0) {
            exclusions.add(new Rect(view.getWidth() - gestures.right, 0, view.getWidth(), view.getHeight()));
        }
        view.setSystemGestureExclusionRects(exclusions);
    }

    private static native void nativeSafeAreaInsets(int left, int top, int right, int bottom);

    private File dataDir() {
        File[] media = getExternalMediaDirs();
        if (media != null && media.length > 0 && media[0] != null
            && (media[0].isDirectory() || media[0].mkdirs())) {
            return media[0];
        }
        return getExternalFilesDir(null);
    }

    private File targetFor(String path, File media) {
        return new File(path.equals(INTERNAL_ROOT) ? getFilesDir() : media, path);
    }

    private void unpackAssets(File media) throws IOException {
        if (media == null) {
            throw new IOException("No app folder");
        }
        if (!media.isDirectory() && !media.mkdirs()) {
            throw new IOException("Could not create " + media);
        }

        String fingerprint = shippedFingerprint();
        File stamp = new File(getFilesDir(), STAMP);
        if (stamp.isFile() && fingerprint.equals(readText(stamp)) && shippedAssetsExist(media)) {
            return;
        }
        stamp.delete();

        long start = System.nanoTime();
        copyShipped(media);
        writeText(stamp, fingerprint);
        Log.i(TAG, "Unpacked shipped assets " + fingerprint + " in " + (System.nanoTime() - start) / 1000000 + " ms");
    }

    private boolean shippedAssetsExist(File media) {
        for (String path : SHIPPED) {
            File file = targetFor(path, media);
            if (!file.exists() || (file.isDirectory() && file.list() == null)) {
                return false;
            }
        }
        return true;
    }

    private String shippedFingerprint() throws IOException {
        List<String> entries = new ArrayList<>();
        try (ZipFile apk = new ZipFile(getApplicationInfo().sourceDir)) {
            for (Enumeration<? extends ZipEntry> e = apk.entries(); e.hasMoreElements();) {
                ZipEntry entry = e.nextElement();
                if (entry.isDirectory()) {
                    continue;
                }
                for (String root : SHIPPED) {
                    String prefix = "assets/" + root;
                    if (entry.getName().equals(prefix) || entry.getName().startsWith(prefix + "/")) {
                        entries.add(entry.getName() + ":" + entry.getSize() + ":" + entry.getCrc());
                        break;
                    }
                }
            }
        }
        Collections.sort(entries);
        CRC32 digest = new CRC32();
        for (String entry : entries) {
            digest.update(entry.getBytes(StandardCharsets.UTF_8));
        }
        return String.format("%08x.%d", digest.getValue(), entries.size());
    }

    private void copyShipped(File media) throws IOException {
        Set<File> made = new HashSet<>();
        byte[] buffer = new byte[64 * 1024];
        try (ZipFile apk = new ZipFile(getApplicationInfo().sourceDir)) {
            for (Enumeration<? extends ZipEntry> e = apk.entries(); e.hasMoreElements();) {
                ZipEntry entry = e.nextElement();
                if (entry.isDirectory() || !entry.getName().startsWith("assets/")) {
                    continue;
                }
                String name = entry.getName().substring("assets/".length());
                for (String root : SHIPPED) {
                    if (!name.equals(root) && !name.startsWith(root + "/")) {
                        continue;
                    }
                    File target = new File(targetFor(root, media).getParentFile(), name);
                    File parent = target.getParentFile();
                    if (parent != null && made.add(parent) && !parent.isDirectory() && !parent.mkdirs()) {
                        throw new IOException("Could not create " + parent);
                    }
                    try (InputStream in = apk.getInputStream(entry); OutputStream out = new FileOutputStream(target)) {
                        int read;
                        while ((read = in.read(buffer)) != -1) {
                            out.write(buffer, 0, read);
                        }
                    }
                    break;
                }
            }
        }
    }

    private static String readText(File file) {
        try (InputStream in = new java.io.FileInputStream(file)) {
            byte[] bytes = new byte[(int) Math.min(file.length(), 64L)];
            int read = in.read(bytes);
            return read <= 0 ? "" : new String(bytes, 0, read, StandardCharsets.UTF_8).trim();
        } catch (IOException e) {
            return "";
        }
    }

    private static void writeText(File file, String text) throws IOException {
        try (OutputStream out = new FileOutputStream(file)) {
            out.write(text.getBytes(StandardCharsets.UTF_8));
        }
    }
}
