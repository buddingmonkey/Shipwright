import com.android.build.api.artifact.SingleArtifact
import java.net.URI

plugins {
    id("com.android.application")
}

val repoRoot: File = rootProject.projectDir.parentFile
val outputRoot: File = repoRoot.resolve("build-android")

layout.buildDirectory.set(outputRoot.resolve("app"))

val sohVersion: String = Regex("""project\(Ship\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)""")
    .find(repoRoot.resolve("CMakeLists.txt").readText())?.groupValues?.get(1)
    ?: error("Could not read the project version from ${repoRoot.resolve("CMakeLists.txt").path}")

val sdl2Src: File = outputRoot.resolve("sdl2-src")
val stagedAssets: File = layout.buildDirectory.dir("soh-assets").get().asFile

android {
    namespace = "com.harbormasters.soh"
    compileSdk = 35
    ndkVersion = "29.0.14206865"

    defaultConfig {
        applicationId = providers.gradleProperty("applicationId").get()
        minSdk = 29
        targetSdk = 35
        versionCode = providers.gradleProperty("versionCode").getOrElse("1").toInt()
        versionName = sohVersion

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DUSE_OPENGLES=ON",
                    "-DENABLE_OPENXR=ON",
                    "-DANDROID_STL=c++_shared",
                    "-DFETCHCONTENT_SOURCE_DIR_SDL2=${sdl2Src.absolutePath}",
                    "-DENABLE_DEBUG_TOOLS=${providers.gradleProperty("debugTools").getOrElse("OFF")}"
                )
                targets += "soh"
            }
        }

        ndk {
            abiFilters += providers.gradleProperty("abis").getOrElse("arm64-v8a,x86_64").split(",")
        }
    }

    externalNativeBuild {
        cmake {
            path = repoRoot.resolve("CMakeLists.txt")
            version = "3.31.6"
            buildStagingDirectory = outputRoot.resolve(".cxx")
        }
    }

    sourceSets.getByName("main") {
        java.srcDir(sdl2Src.resolve("android-project/app/src/main/java"))
        assets.srcDir(stagedAssets)
    }

    val keystoreFile = providers.gradleProperty("keystoreFile").orNull
    if (keystoreFile != null) {
        signingConfigs.create("sideload") {
            storeFile = file(keystoreFile)
            storePassword = providers.gradleProperty("keystorePassword").get()
            keyAlias = providers.gradleProperty("keyAlias").get()
            keyPassword = providers.gradleProperty("keyPassword").get()
            enableV2Signing = true
            enableV3Signing = true
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            isJniDebuggable = false
            signingConfig = if (keystoreFile != null) {
                signingConfigs.getByName("sideload")
            } else {
                signingConfigs.getByName("debug")
            }
        }
        debug {
            isJniDebuggable = true
            externalNativeBuild {
                cmake {
                    arguments += "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
                }
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    packaging {
        jniLibs.useLegacyPackaging = false
    }

    androidResources {
        noCompress += "o2r"
    }
}

val fetchGameControllerDb by tasks.registering {
    val out = stagedAssets.resolve("gamecontrollerdb.txt")
    outputs.file(out)
    doLast {
        if (out.exists() && out.length() > 0L) return@doLast
        out.parentFile.mkdirs()
        try {
            val url = "https://raw.githubusercontent.com/mdqinc/SDL_GameControllerDB/master/gamecontrollerdb.txt"
            out.writeBytes(URI(url).toURL().readBytes())
        } catch (e: Exception) {
            logger.warn("Could not download gamecontrollerdb.txt (${e.message}); shipping without it")
            out.writeText("")
        }
    }
}

val hostTools: File = repoRoot.resolve("build-android/host-tools")
val hostSohO2r: File = hostTools.resolve("soh/soh.o2r")

val configureHostTools by tasks.registering(Exec::class) {
    onlyIf { !hostTools.resolve("CMakeCache.txt").exists() }
    commandLine(
        "cmake", "-S", repoRoot.path, "-B", hostTools.path,
        "-DSOH_TOOLS_ONLY=ON", "-DCMAKE_BUILD_TYPE=Release"
    )
}

val generateSohOtr by tasks.registering(Exec::class) {
    dependsOn(configureHostTools)
    commandLine("cmake", "--build", hostTools.path, "--target", "GenerateSohOtr", "--parallel")
    outputs.upToDateWhen { false }
}

val stageSohAssets by tasks.registering(Copy::class) {
    dependsOn(generateSohOtr)
    into(stagedAssets)
    from(hostSohO2r)
    from(repoRoot.resolve("soh/assets/yml")) { into("assets") }
    doFirst {
        if (!hostSohO2r.exists() || hostSohO2r.length() == 0L) {
            throw GradleException("GenerateSohOtr left no soh.o2r at ${hostSohO2r.path}")
        }
    }
}

tasks.named("preBuild") {
    dependsOn(stageSohAssets, fetchGameControllerDb)
}

androidComponents {
    onVariants { variant ->
        val suffix = variant.name.replaceFirstChar { it.uppercase() }
        val apkDir = variant.artifacts.get(SingleArtifact.APK)
        val target = outputRoot.resolve("soh-${variant.name}.apk")
        val publish = tasks.register("publish${suffix}Apk") {
            outputs.file(target)
            outputs.upToDateWhen { false }
            doLast {
                val dir = apkDir.get().asFile
                val apk = dir.listFiles { f: File -> f.extension == "apk" }?.firstOrNull()
                    ?: throw GradleException("No APK was produced in $dir")
                apk.copyTo(target, overwrite = true)
            }
        }
        tasks.matching { it.name == "assemble$suffix" }.configureEach { finalizedBy(publish) }
    }
}
