
package jp.mack.xposedinjectdart;

import android.app.Application;
import android.content.SharedPreferences;
import android.util.Log;

import io.github.libxposed.service.XposedService;
import io.github.libxposed.service.XposedServiceHelper;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class ConfigApplication extends Application
        implements XposedServiceHelper.OnServiceListener {

    private static final String TAG = "DartPatchConfig";

    public static final String LOCAL_PREFS = "coordinate_config";
    public static final String REMOTE_PREFS = "pentan";

    public static final String KEY_X = "coordinate_x";
    public static final String KEY_Y = "coordinate_y";

    private volatile XposedService xposedService;

    private final ExecutorService executor =
            Executors.newSingleThreadExecutor();

    @Override
    public void onCreate() {
        super.onCreate();

        // Xposed Serviceへの接続を登録
        XposedServiceHelper.registerListener(this);
    }

    @Override
    public void onServiceBind(XposedService service) {
        Log.i(TAG, "XposedService connected");

        xposedService = service;

        // 接続時にローカル設定をRemote Preferencesへ同期
        executor.execute(() -> syncCoordinates(service));
    }

    @Override
    public void onServiceDied(XposedService service) {
        Log.w(TAG, "XposedService disconnected");

        if (xposedService == service) {
            xposedService = null;
        }
    }

    public void saveCoordinates(float x, float y) {

        // まずモジュール自身のSharedPreferencesへ保存
        getSharedPreferences(LOCAL_PREFS, MODE_PRIVATE)
                .edit()
                .putFloat(KEY_X, x)
                .putFloat(KEY_Y, y)
                .apply();

        // Serviceが接続済みならRemote Preferencesにも同期
        XposedService service = xposedService;

        if (service != null) {
            executor.execute(() -> syncCoordinates(service));
        }
    }

    private void syncCoordinates(XposedService service) {
        try {
            SharedPreferences local =
                    getSharedPreferences(LOCAL_PREFS, MODE_PRIVATE);

            float x = local.getFloat(KEY_X, -20.0f);
            float y = local.getFloat(KEY_Y, 250.0f);

            SharedPreferences remote =
                    service.getRemotePreferences(REMOTE_PREFS);

            SharedPreferences.Editor editor = remote.edit();

            if (editor == null) {
                throw new IllegalStateException(
                        "RemotePreferences editor unavailable");
            }

            editor.putFloat(KEY_X, x);
            editor.putFloat(KEY_Y, y);

            if (!editor.commit()) {
                throw new IllegalStateException(
                        "RemotePreferences commit failed");
            }

            Log.i(TAG, "Coordinates synced: x=" + x + ", y=" + y);

        } catch (Throwable e) {
            Log.e(TAG, "Failed to sync coordinates", e);
        }
    }
}
