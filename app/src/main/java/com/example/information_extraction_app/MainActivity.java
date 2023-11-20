package com.example.information_extraction_app;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.example.information_extraction_app.databinding.ActivityMainBinding;

import org.w3c.dom.Text;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'information_extraction_app' library on application startup.
    static {
        System.loadLibrary("information_extraction_app");
    }

    private ActivityMainBinding binding;

    private static final String TAG = "MyActivity";




    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
//        String res = stringFromJNI("无冠脉风险,没栓塞风险,鱼精蛋白剂量是300m,出血量为20ml,30ml造影剂, 麻醉方式为全麻,血管入路为经右股动脉，入路方式为超声评估");
//        tv.setText(res);



        Button btn1, btn2;
        btn1 = (Button)findViewById(R.id.button);
        btn1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    AssetManager assetManager = getAssets();
//                    InputStream inputStream = getAssets().open("label_flag.txt"); // 从assets目录读取文件
//                    int length = inputStream.available();
//                    BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
//                    String line;
//                    ArrayList<String> lines = new ArrayList<>();
//                    while ((line = reader.readLine()) != null) {
//                        lines.add(line); // 逐行存储字段
//                        System.out.println(line);
//                    }
//                    // 现在您有了存储字段的ArrayList
//                    String[] stringArray = lines.toArray(new String[lines.size()]);
//                    int numLines = stringArray.length;
//                    String res = loadModel( "label_flag", stringArray, numLines);
//                    Log.d(TAG, res);
//                    tv.setText(res);
                    String res = loadModel(assetManager);
                    tv.setText(res);
                } catch (Exception e) {
                    e.printStackTrace();
                }

            }
        });

        btn2 = (Button) findViewById(R.id.button2);
        btn2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                EditText editText = findViewById(R.id.editTextText);
                String sentence = editText.getText().toString();
                String res = stringFromJNI(sentence);

                tv.setText(res);

            }
        });
    }

    /**
     * A native method that is implemented by the 'information_extraction_app' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI(String str);

    public native String loadModel(AssetManager assetManager);
}