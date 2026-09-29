pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "Ship of Harkinian"
include(":app")

val sdl2Tag: String = rootDir.parentFile.resolve("libultraship/cmake/dependencies/android.cmake").readText()
    .let { Regex("""FetchContent_Declare\(\s*SDL2\s[^)]*GIT_TAG\s+(\S+)""").find(it)?.groupValues?.get(1) }
    ?: error("No SDL2 GIT_TAG in libultraship/cmake/dependencies/android.cmake")

val sdl2Patch = rootDir.resolve("sdl2-receiver-exported.patch")
val sdl2Version = sdl2Tag + " " + java.security.MessageDigest.getInstance("SHA-256")
    .digest(sdl2Patch.readBytes()).joinToString("") { "%02x".format(it) }

val sdl2Dir = rootDir.parentFile.resolve("build-android/sdl2-src")
val sdl2Stamp = sdl2Dir.resolve(".soh-sdl2-tag")
if (!sdl2Dir.resolve("android-project").isDirectory ||
    !sdl2Stamp.isFile ||
    sdl2Stamp.readText().trim() != sdl2Version
) {
    sdl2Dir.parentFile.mkdirs()
    sdl2Dir.deleteRecursively()
    logger.lifecycle("Cloning SDL2 $sdl2Tag into ${sdl2Dir.path}")
    val result = providers.exec {
        commandLine(
            "git", "clone", "--depth", "1", "--branch", sdl2Tag,
            "https://github.com/libsdl-org/SDL.git", sdl2Dir.absolutePath
        )
        isIgnoreExitValue = true
    }
    if (result.result.get().exitValue != 0) {
        error("Failed to clone SDL2 $sdl2Tag:\n${result.standardError.asText.get()}")
    }
    val patched = providers.exec {
        commandLine("git", "-C", sdl2Dir.absolutePath, "apply", sdl2Patch.absolutePath)
        isIgnoreExitValue = true
    }
    if (patched.result.get().exitValue != 0) {
        error("Failed to apply ${sdl2Patch.name} to SDL2 $sdl2Tag:\n${patched.standardError.asText.get()}")
    }
    sdl2Stamp.writeText(sdl2Version)
}
