#include <cstdlib>
#include <android/log.h>
#include <sys/system_properties.h>
#include "zygisk.hpp"

// Define a logging tag so you can read your module's output in Logcat
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "FPS_Unlocker", __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

class FpsUnlockerModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    // This runs BEFORE the app fully opens. We use it to check the package name.
    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        
        // Target specific game package names (e.g., PUBG, CODM)
        if (strcmp(process, "com.tencent.ig") == 0 || 
            strcmp(process, "com.activision.callofduty.shooter") == 0) {
            
            LOGD("Target game detected: %s. Preparing to spoof properties...", process);
            is_target_game = true;
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    // This runs AFTER the app's process is created. We apply the spoofing here.
    void postAppSpecialize(const AppSpecializeArgs *args) override {
        if (is_target_game) {
            LOGD("Injecting ROG Phone 6 properties...");
            
            // Note: Actual memory hooking of __system_property_get goes here.
            // You would use your Api* to hook the property retrieval functions
            // and force them to return:
            // ro.product.manufacturer = "asus"
            // ro.product.model = "ASUS_AI2201_A"
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool is_target_game = false;
};

// Register the module with Zygisk
REGISTER_ZYGISK_MODULE(FpsUnlockerModule)
/* Copyright 2022-2023 John "topjohnwu" Wu
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>

#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "MyModule", __VA_ARGS__)

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        // Use JNI to fetch our process name
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        preSpecialize(process);
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void preServerSpecialize(ServerSpecializeArgs *args) override {
        preSpecialize("system_server");
    }

private:
    Api *api;
    JNIEnv *env;

    void preSpecialize(const char *process) {
        // Demonstrate connecting to to companion process
        // We ask the companion for a random number
        unsigned r = 0;
        int fd = api->connectCompanion();
        read(fd, &r, sizeof(r));
        close(fd);
        LOGD("process=[%s], r=[%u]\n", process, r);

        // Since we do not hook any functions, we should let Zygisk dlclose ourselves
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

};

static int urandom = -1;

static void companion_handler(int i) {
    if (urandom < 0) {
        urandom = open("/dev/urandom", O_RDONLY);
    }
    unsigned r;
    read(urandom, &r, sizeof(r));
    LOGD("companion r=[%u]\n", r);
    write(i, &r, sizeof(r));
}

// Register our module class and the companion handler function
REGISTER_ZYGISK_MODULE(MyModule)
REGISTER_ZYGISK_COMPANION(companion_handler)
