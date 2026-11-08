
package org.modfs.xposedmenu;

import android.os.Build;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

import io.github.libxposed.api.XposedModule;
import io.github.libxposed.api.XposedModuleInterface;

public class Inject extends XposedModule {

    private static final String TAG = "DartPatch";
    private static final Map<String, String> LIBRARIES = new HashMap<>();

    static {
        LIBRARIES.put(
            "jp.zaizen.colorplace",
            "libcolorplace.so"
        );

        LIBRARIES.put(
            "jp.zaizen.crossmath",
            "libcrossmath.so"
        );

        LIBRARIES.put(
            "jp.zaizen.klondike",
            "libklondike.so"
        );
        LIBRARIES.put(
            "jp.zaizen.left10",
            "libleft10.so"
        );

        LIBRARIES.put(
            "jp.zaizen.minesweeper",
            "libminesweeper.so"
        );

        LIBRARIES.put(
            "jp.zaizen.multiplication",
            "libmultiplication.so"
        );
        LIBRARIES.put(
            "jp.zaizen.numbersum",
            "libnumbersum.so"
        );

        LIBRARIES.put(
            "jp.zaizen.wordpuzzle",
            "libwordpuzzle.so"
        );

        LIBRARIES.put(
            "jp.zaizen.other",
            "libotherapp.so"
        );        
    }

    public Inject() {
        super();
    }

    @Override
    public void onPackageLoaded(
            XposedModuleInterface.PackageLoadedParam param) {

        String packageName = param.getPackageName();

        String libraryName = LIBRARIES.get(packageName);

        if (libraryName == null) {
            return;
        }

        Log.i(TAG, "Target package: " + packageName);
        Log.i(TAG, "Selected library: " + libraryName);

        loadNativeLibrary(packageName, libraryName);
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
