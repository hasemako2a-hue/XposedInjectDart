
package org.modfs.xposedmenu;

import android.os.Build;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;

import io.github.libxposed.api.XposedModule;
import io.github.libxposed.api.XposedModuleInterface;

public class Inject extends XposedModule {

    private static final String TAG = "DartPatch";
    private static final String TARGET_PACKAGE =
            "jp.zaizen.wordpuzzle";

    private static boolean injected = false;

    public Inject() {
        super();
    }

    @Override
    public void onPackageLoaded(
            @NonNull PackageLoadedParam param) {

        super.onPackageLoaded(param);

        String packageName = param.getPackageName();
        Log.i(TAG,packageName);        

        // 対象アプリ以外は処理しない
        if (!TARGET_PACKAGE.equals(packageName)) {
            return;
        }

        if (injected) {
            return;
        }

        injected = true;
        Log.i(TAG,"start injection"); 
        try {
            String abi = Build.SUPPORTED_ABIS[0];
            String resourceName = "lib/" + abi + "/libxposedmenu.so";
            InputStream input = Inject.class.getClassLoader().getResourceAsStream(resourceName);

            if (input == null) {
                throw new Exception("Native library not found: " + resourceName);
            }
            Log.i(TAG,"InputStream found"); 
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] temp = new byte[8192];
            int count;

            while ((count = input.read(temp)) != -1) {
                buffer.write(temp, 0, count);
            }

            input.close();
            Log.i(TAG,"finish InputStream"); 
            File soFile = new File("/data/user/0/" + packageName + "/cache/libxposedmenu.so");
            File parent = soFile.getParentFile();

            if (parent != null && !parent.exists()) {
                if (!parent.mkdirs() && !parent.exists()) {
                    throw new Exception("Cannot create cache directory");
                }
            }
            Log.i(TAG,"file exist"); 

            try (FileOutputStream output = new FileOutputStream(soFile, false)) {
                buffer.writeTo(output);
                output.flush();
            }

            // 絶対パスでネイティブライブラリをロード
            System.load(soFile.getAbsolutePath());
            log(Log.INFO, TAG, "Loaded native library into " + packageName);

        } catch (Throwable t) {
            injected = false;
            log(Log.ERROR, TAG,"Native injection failed: " + t);
            log(Log.ERROR, TAG, Log.getStackTraceString(t));
        }
    }
}
