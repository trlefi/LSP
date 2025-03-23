package Xp.YuJian.App.工具类;

import Xp.YuJian.App.光遇.Hook.三服配置区;
import Xp.YuJian.App.工具类.启动器配置.全局载入;
import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;
import androidx.annotation.RequiresApi;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import org.json.JSONObject;

public class 二进制执行 {

    public interface CppExecutionCallback {
        void onExecutionResult(String result);
    }

    public static void cpp(final String name, final Activity activity, final CppExecutionCallback callback, final String content) {
        new Thread(new Runnable() {
            @RequiresApi(api = Build.VERSION_CODES.R)
            public void run() {
                try {
                    File filesDir = activity.getFilesDir();
                    String binaryFilePath = filesDir.getAbsolutePath() + "/lib/" + name;
                    String contentFilePath = filesDir.getAbsolutePath() + "/" + 三服配置区.应用名称 + "配置/执行.txt";

                    File contentDirectory = new File(filesDir.getAbsolutePath() + "/" + 三服配置区.应用名称 + "配置");
                    if (!contentDirectory.exists()) {
                        contentDirectory.mkdirs();
                    }

                    FileOutputStream fos = new FileOutputStream(contentFilePath);

                    String deviceCode = 三服配置区.Android_id;
                    String denglu = "设备码" + deviceCode + "设备码结束" + "卡密" + 三服配置区.账号 + "卡密结束" + "时间戳开始" + 获取时间戳() + "时间戳结束";

                    String 卡密 = null;
                    String 加密 = null;
                    String binaryData = null;

                    try {
                        卡密 = aes.Encrypt(denglu, 三服配置区.aeskey);
                        String kami = denglu;

                        JSONObject jsonObject = new JSONObject();
                        jsonObject.put("content", content);
                        jsonObject.put("cpptimestamp", 获取时间戳());
                        jsonObject.put("data", kami);

                        String zhuhe = jsonObject.toString();

                        加密 = aes.Encrypt(zhuhe, 三服配置区.aeskey);
                        binaryData = 转进制(加密);
                    } catch (Exception e) {
                        Log.e("BinaryExecution", "加密错误", e);
                    }

                    if (三服配置区.bao.equals("com.tgc.sky.android")) {
                        fos.write(content.getBytes());
                    } else {
                        fos.write(binaryData.getBytes());
                    }
                    fos.close();

                    // 记录执行内容
                    String record = generateRecord(content);
                    synchronized (二进制执行.class) {
                        全局载入.执行功能 += record + "\n";
                    }

                    File contentFile = new File(contentFilePath);
                    contentFile.setReadable(true, false);
                    contentFile.setWritable(true, false);

                    long startTime = System.currentTimeMillis();
                    boolean success = false;
                    do {
                        Process process;
                        int targetSdkVersion = 0;

                        try {
                            ApplicationInfo applicationInfo = activity.getPackageManager().getApplicationInfo(activity.getPackageName(), 0);
                            targetSdkVersion = applicationInfo.targetSdkVersion;
                        } catch (PackageManager.NameNotFoundException e) {
                            Log.e("BinaryExecution", "获取targetSdkVersion失败", e);
                        }

                        if (targetSdkVersion > 28 && Build.VERSION.SDK_INT > Build.VERSION_CODES.Q) { // targetSdkVersion > 28 and Android 11 及以上
                            process = Runtime.getRuntime().exec(new String[]{"su", "-c", binaryFilePath});
                        } else {
                            process = Runtime.getRuntime().exec(binaryFilePath);
                        }

                        BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()));
                        BufferedReader stdError = new BufferedReader(new InputStreamReader(process.getErrorStream()));

                        StringBuilder output = new StringBuilder();
                        String s;
                        while ((s = stdInput.readLine()) != null) {
                            output.append(s).append("\n");
                        }

                        StringBuilder error = new StringBuilder();
                        while ((s = stdError.readLine()) != null) {
                            error.append(s).append("\n");
                        }

                        if (content.equals("ydpt") || content.equals("jspt") || content.equals("sdptmax") || content.equals("hddb") || content.equals("ydxg")) {
                            long endTime = System.currentTimeMillis();
                            success = (endTime - startTime >= 10000);
                        } else {
                            success = output.toString().contains("完成") || output.toString().contains("错误");
                        }

                        if (success) {
                            final String result = "执行结果: " + output + "\nError: " + error;
                            activity.runOnUiThread(new Runnable() {
                                public void run() {
                                    callback.onExecutionResult(result);
                                }
                            });
                        }
                    } while (!success);

                } catch (IOException e) {
                    Log.e("BinaryExecution", "IO错误", e);
                }
            }
        }).start();
    }


    // 生成记录
    private static String generateRecord(String content) {
        String description = "";
        switch (content) {
            case "ydpt":
                description = "原地跑图";
                break;
            case "jspt":
                description = "极速跑图";
                break;
            case "sdptmax":
                description = "超速跑图";
                break;
            case "rw1":
                description = "任务①";
                break;
            case "rw2":
                description = "任务②";
                break;
            case "rw3":
                description = "任务③";
                break;
            case "rw4":
                description = "任务④";
                break;
            case "jsk":
                description = "修改速度";
                break;
            case "tiaozhuang":
                description = "跳转位置";
                break;
            case "dqwz":
                description = "获取位置";
                break;
            case "cspt":
                description = "传送跑图";
                break;
            case "hddb":
                description = "获取代币";
                break;
            case "chuansong":
                description = "传送地图";
                break;
            case "ydxg":
                description = "原地霞谷";
                break;
            case "zhk":
                description = "炸花开";
                break;
            case "zhg":
                description = "炸花关";
                break;
            case "khtk":
                description = "卡后台开";
                break;
            case "khtg":
                description = "卡后台关";
                break;
            case "ysk":
                description = "隐身开";
                break;
            case "ysg":
                description = "隐身关";
                break;
            case "xaxhk":
                description = "吸火开";
                break;
            case "xaxhg":
                description = "吸火关";
                break;
            case "cxsg":
                description = "查询身高";
                break;
            case "xslz":
                description = "显示隐藏蜡烛";
                break;
            case "xzbl":
                description = "先祖白蜡";
                break;
            case "qbdz":
                description = "全部动作";
                break;
            case "jjxz":
                description = "季节先祖";
                break;
            case "xzbl":
                description = "先祖白蜡";
                break;
            case "ydkt":
                description = "原地开图";
                break;
            case "hysg":
                description = "好友身高";
                break;
            case "djmsk":
                description = "单机模式开";
                break;
            case "djmsg":
                description = "单机模式关";
                break;
            case "songhuo":
                description = "送火";
                break;
            case "shouhuo":
                description = "收火";
                break;
            case "shouxin":
                description = "收心";
                break;
            case "qygk":
                description = "全衣柜开";
                break;
            case "qygg":
                description = "全衣柜关";
                break;
            case "hqjl":
                description = "获取季蜡";
                break;
            case "tdhk":
                description = "跳过动画开";
                break;
            case "tdhg":
                description = "跳过动画关";
                break;
            case "wxnlk":
                description = "无限能量开";
                break;
            case "wxnlg":
                description = "无限能量关";
                break;
            case "ydxj":
                description = "无限能量关";
                break;
            case "ydgy":
                description = "原地光翼";
                break;
            case "yjxj":
                description = "一键献祭";
                break;
            default:
                description = content;
                break;
        }
        SimpleDateFormat sdf = new SimpleDateFormat("-HH时mm分ss秒-", Locale.getDefault());
        String timestamp = sdf.format(new Date());
        return timestamp + "【" + description + "】";
    }

    public static String 转进制(String input) {
        StringBuilder binaryString = new StringBuilder();
        for (char c : input.toCharArray()) {
            String binaryChar = String.format("%8s", Integer.toBinaryString(c)).replace(' ', '0');
            binaryString.append(binaryChar);
        }
        return binaryString.toString();
    }

    public static String 获取时间戳() {
        long timestampMillis = System.currentTimeMillis();
        long timestampSeconds = timestampMillis / 1000;
        return String.valueOf(timestampSeconds);
    }
}
