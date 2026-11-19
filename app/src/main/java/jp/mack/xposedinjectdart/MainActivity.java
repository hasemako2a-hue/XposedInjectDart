
package jp.mack.xposedinjectdart;

import android.os.Bundle;
import android.text.InputType;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private EditText editX;
    private EditText editY;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(32, 32, 32, 32);

        TextView title = new TextView(this);
        title.setText("pentan 座標設定");
        title.setTextSize(22);
        layout.addView(title);

        editX = createCoordinateInput("X座標");
        editY = createCoordinateInput("Y座標");

        layout.addView(editX);
        layout.addView(editY);

        // 保存済みの値を読み込む
        var prefs = getSharedPreferences(
                ConfigApplication.LOCAL_PREFS,
                MODE_PRIVATE
        );

        editX.setText(Float.toString(
                prefs.getFloat(ConfigApplication.KEY_X, -20.0f)));

        editY.setText(Float.toString(
                prefs.getFloat(ConfigApplication.KEY_Y, 250.0f)));

        Button saveButton = new Button(this);
        saveButton.setText("保存");

        saveButton.setOnClickListener(v -> saveCoordinates());

        layout.addView(saveButton);

        setContentView(layout);
    }

    private EditText createCoordinateInput(String hint) {
        EditText edit = new EditText(this);

        edit.setHint(hint);
        edit.setSingleLine(true);
        edit.setInputType(
                InputType.TYPE_CLASS_NUMBER
                        | InputType.TYPE_NUMBER_FLAG_SIGNED
                        | InputType.TYPE_NUMBER_FLAG_DECIMAL
        );

        edit.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        ));

        return edit;
    }

    private void saveCoordinates() {
        try {
            float x = Float.parseFloat(editX.getText().toString());
            float y = Float.parseFloat(editY.getText().toString());

            if (!Float.isFinite(x) || !Float.isFinite(y)) {
                throw new NumberFormatException();
            }

            ConfigApplication app =
                    (ConfigApplication) getApplication();

            app.saveCoordinates(x, y);

            Toast.makeText(
                    this,
                    "座標を保存しました",
                    Toast.LENGTH_SHORT
            ).show();

        } catch (NumberFormatException e) {
            Toast.makeText(
                    this,
                    "数値を正しく入力してください",
                    Toast.LENGTH_SHORT
            ).show();
        }
    }
}
