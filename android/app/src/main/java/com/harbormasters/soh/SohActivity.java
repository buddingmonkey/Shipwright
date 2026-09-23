package com.harbormasters.soh;

import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
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
import java.util.List;
import java.util.zip.CRC32;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import android.content.res.AssetManager;

import org.libsdl.app.SDLActivity;

public class SohActivity extends SDLActivity {
    private static final String TAG = "SoH";
    private static final String STAMP = ".unpacked";

    private static final String[] SHIPPED = {
        "soh.o2r",
        "assets",
        "gamecontrollerdb.txt",
    };

    private volatile File dataDir;

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        dataDir = dataDir();
        try {
            unpackAssets(dataDir);
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
        super.onDestroy();
        System.exit(0);
    }

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

    private void unpackAssets(File target) throws IOException {
        if (target == null) {
            throw new IOException("No app folder");
        }
        if (!target.isDirectory() && !target.mkdirs()) {
            throw new IOException("Could not create " + target);
        }

        String fingerprint = shippedFingerprint();
        File stamp = new File(target, STAMP);
        if (stamp.isFile() && fingerprint.equals(readText(stamp)) && shippedAssetsExist(target)) {
            return;
        }
        stamp.delete();

        AssetManager assets = getAssets();
        for (String path : SHIPPED) {
            copyAsset(assets, path, new File(target, path));
        }
        writeText(stamp, fingerprint);
        Log.i(TAG, "Unpacked shipped assets " + fingerprint);
    }

    private boolean shippedAssetsExist(File target) {
        for (String path : SHIPPED) {
            File file = new File(target, path);
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

    private void copyAsset(AssetManager assets, String path, File target) throws IOException {
        String[] children = assets.list(path);
        if (children != null && children.length > 0) {
            if (!target.isDirectory() && !target.mkdirs()) {
                throw new IOException("Could not create " + target);
            }
            for (String child : children) {
                copyAsset(assets, path + "/" + child, new File(target, child));
            }
            return;
        }

        File parent = target.getParentFile();
        if (parent != null && !parent.isDirectory() && !parent.mkdirs()) {
            throw new IOException("Could not create " + parent);
        }
        try (InputStream in = assets.open(path); OutputStream out = new FileOutputStream(target)) {
            byte[] buffer = new byte[64 * 1024];
            int read;
            while ((read = in.read(buffer)) != -1) {
                out.write(buffer, 0, read);
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
