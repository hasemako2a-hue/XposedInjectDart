
package jp.mack.xposedinjectdart;

import android.os.Build;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import io.github.libxposed.api.XposedModule;
import io.github.libxposed.api.XposedModuleInterface;

public class Inject extends XposedModule {

    private static final String TAG = "DartPatch";
    private static final String[] apps = {"colorplace", "crossmath", "jyanken", "klondike", "left10", "minesweeper", "multiplication", "numbersum", "tilematch", "wordpuzzle"};

    public Inject() {
        super();
    }

    @Override
    public void onPackageLoaded(XposedModuleInterface.PackageLoadedParam param) {

        String packageName = param.getPackageName();
        if (packageName.startsWith("jp.zaizen")) {
            String appName = packageName.substring(10);
            if (Arrays.asList(apps).contains(appName)) {
                Log.i(TAG, "Target package: " + packageName);
                loadNativeLibrary(packageName, String.format("lib%s.so", appName));
            }
        }

        
    }

    private void loadNativeLibrary(
            String packageName,
            String libraryName) {

        String abi = Build.SUPPORTED_ABIS[0];
        String resourcePath = "lib/" + abi + "/" + libraryName;

        try {
            InputStream input = Inject.class.getClassLoader().getResourceAsStream(resourcePath);

            if (input == null) {
                Log.e(TAG, "Library resource not found: " + resourcePath);
                return;
            }

            ByteArrayOutputStream output = new ByteArrayOutputStream();

            byte[] buffer = new byte[8192];
            int length;

            try (InputStream in = input) {
                while ((length = in.read(buffer)) != -1) {
                    output.write(buffer, 0, length);
                }
            }

            File cacheDir = new File("/data/user/0/" + packageName + "/cache");

            if (!cacheDir.exists() && !cacheDir.mkdirs()) {
                Log.e(TAG, "Failed to create cache directory");
                return;
            }

            File libraryFile = new File(cacheDir, libraryName);

            try (FileOutputStream fos = new FileOutputStream(libraryFile)) {
                fos.write(output.toByteArray());
            }

            System.load(libraryFile.getAbsolutePath());

            Log.i(TAG, "Loaded " + libraryName + " into " + packageName);

        } catch (Throwable e) {
            Log.e(TAG, "Failed to load " + libraryName, e);
        }
    }
}
