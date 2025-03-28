package Xp.YuJian.App;

import Xp.YuJian.App.光遇.Hook.三服配置区;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toolbar;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(createLayout());
    }

    private View createLayout() {
		Context context = this;

		LinearLayout.LayoutParams matchParentParams =
            new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

		LinearLayout mainLayout = new LinearLayout(context);
		mainLayout.setOrientation(LinearLayout.VERTICAL);
		mainLayout.setLayoutParams(matchParentParams);
		mainLayout.setBackgroundColor(Color.WHITE);

		Toolbar toolbar = new Toolbar(context);
		toolbar.setTitle(三服配置区.应用名称); 
		toolbar.setBackgroundColor(getResources().getColor(R.color.colorPrimary)); // 设置工具栏的背景颜色
		toolbar.setTitleTextColor(Color.WHITE);
		mainLayout.addView(toolbar);


		ScrollView scrollView = new ScrollView(context);
		scrollView.setLayoutParams(matchParentParams);

		LinearLayout innerLayout = new LinearLayout(context);
		innerLayout.setOrientation(LinearLayout.VERTICAL);
		innerLayout.setLayoutParams(matchParentParams);
		innerLayout.setPadding(16, 16, 16, 16);

		LinearLayout linear2 = new LinearLayout(context);
		linear2.setOrientation(LinearLayout.VERTICAL);
		linear2.setLayoutParams(new LinearLayout.LayoutParams(
									ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear2.setPadding(8, 8, 8, 8);
		innerLayout.addView(linear2);

		LinearLayout linear3 = new LinearLayout(context);
		linear3.setOrientation(LinearLayout.VERTICAL);
		linear3.setLayoutParams(new LinearLayout.LayoutParams(
									ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear2.addView(linear3);

		LinearLayout linear4 = new LinearLayout(context);
		linear4.setOrientation(LinearLayout.HORIZONTAL);
		linear4.setLayoutParams(new LinearLayout.LayoutParams(
									ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear4.setPadding(12, 12, 12, 12);
		linear4.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					String url = 三服配置区.URL;
					Intent intent = new Intent(Intent.ACTION_VIEW);
					intent.setData(Uri.parse(url));
					startActivity(intent);
				}
			});
		linear3.addView(linear4);

		TextView textView1 = new TextView(context);
		LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
		layoutParams.weight = 1f;
		textView1.setLayoutParams(layoutParams);
		textView1.setPadding(8, 8, 8, 8);
		textView1.setText("模块更新地址\n游戏内部提示模块更新请在这里更新");
		textView1.setTextSize(12);
		textView1.setTextColor(Color.BLACK);
		linear4.addView(textView1);

		TextView textView2 = new TextView(context);
		textView2.setLayoutParams(new LinearLayout.LayoutParams(
									  ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT));
		textView2.setPadding(8, 8, 8, 8);
		textView2.setText("前往");
		textView2.setTextSize(12);
		textView2.setTextColor(Color.parseColor("#F44336"));
		textView2.setGravity(Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL);
		linear4.addView(textView2);

		View fengexian = new View(context);
		fengexian.setLayoutParams(new LinearLayout.LayoutParams(
									  ViewGroup.LayoutParams.MATCH_PARENT, 1));
		fengexian.setBackgroundColor(Color.parseColor("#E0E0E0"));
		linear3.addView(fengexian);

		LinearLayout linear5 = new LinearLayout(context);
		linear5.setOrientation(LinearLayout.VERTICAL);
		linear5.setLayoutParams(new LinearLayout.LayoutParams(
									ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear5.setPadding(8, 8, 8, 8);
		linear2.addView(linear5);

		LinearLayout linear6 = new LinearLayout(context);
		linear6.setOrientation(LinearLayout.HORIZONTAL);
		linear6.setLayoutParams(new LinearLayout.LayoutParams(
									ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear6.setPadding(12, 12, 12, 12);
		linear5.addView(linear6);

		TextView textView4 = new TextView(context);
		textView4.setLayoutParams(new LinearLayout.LayoutParams(
									  ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
		textView4.setPadding(8, 8, 8, 8);
		textView4.setText("免root请使用容器或者直装版本");
		textView4.setTextSize(12);
		textView4.setTextColor(Color.BLACK);
		linear6.addView(textView4);

		View textView3 = new View(context);
		textView3.setLayoutParams(new LinearLayout.LayoutParams(
									  ViewGroup.LayoutParams.MATCH_PARENT, 1));
		textView3.setBackgroundColor(Color.parseColor("#E0E0E0"));
		linear5.addView(textView3);

		LinearLayout linear7 = new LinearLayout(context);
		linear7.setOrientation(LinearLayout.VERTICAL);
		linear7.setLayoutParams(new LinearLayout.LayoutParams(
									ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear7.setPadding(12, 12, 12, 12);
		linear2.addView(linear7);

		LinearLayout linear8 = new LinearLayout(context);
		linear8.setOrientation(LinearLayout.HORIZONTAL);
		linear8.setLayoutParams(new LinearLayout.LayoutParams(
									ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear8.setPadding(12, 12, 12, 12);
		linear7.addView(linear8);

		TextView textView7 = new TextView(context);
		textView7.setLayoutParams(new LinearLayout.LayoutParams(
									  ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
		textView7.setPadding(8, 8, 8, 8);
		textView7.setText("当前适配光遇");
		textView7.setTextSize(12);
		textView7.setTextColor(Color.BLACK);
		linear8.addView(textView7);

		TextView textView8 = new TextView(context);
		textView8.setLayoutParams(new LinearLayout.LayoutParams(
									  ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		textView8.setPadding(8, 8, 8, 8);
		textView8.setText(三服配置区.游戏版本);
		textView8.setTextSize(12);
		textView8.setTextColor(Color.parseColor("#F44336"));
		linear8.addView(textView8);

		View textView6 = new View(context);
		textView6.setLayoutParams(new LinearLayout.LayoutParams(
									  ViewGroup.LayoutParams.MATCH_PARENT, 1));
		textView6.setBackgroundColor(Color.parseColor("#E0E0E0"));
		linear7.addView(textView6);

		LinearLayout linear9 = new LinearLayout(context);
		linear9.setOrientation(LinearLayout.VERTICAL);
		linear9.setLayoutParams(new LinearLayout.LayoutParams(
									ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear9.setPadding(12, 12, 12, 12);
		linear2.addView(linear9);

		LinearLayout linear10 = new LinearLayout(context);
		linear10.setOrientation(LinearLayout.HORIZONTAL);
		linear10.setLayoutParams(new LinearLayout.LayoutParams(
									 ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear10.setPadding(12, 12, 12, 12);
		linear9.addView(linear10);

		TextView textView10 = new TextView(context);
		textView10.setLayoutParams(new LinearLayout.LayoutParams(
									   ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
		textView10.setPadding(12, 12, 12, 12);
		textView10.setText("这是一个来自小白的编写\n所以可能不能及时更新\n所以希望大家理解\n\n会更新，只是技术不太好\n所以更新很慢");
		textView10.setTextSize(12);
		textView10.setTextColor(Color.BLACK);
		linear10.addView(textView10);

		View textView9 = new View(context);
		textView9.setLayoutParams(new LinearLayout.LayoutParams(
									  ViewGroup.LayoutParams.MATCH_PARENT, 1));
		textView9.setBackgroundColor(Color.parseColor("#E0E0E0"));
		linear9.addView(textView9);

		LinearLayout linear11 = new LinearLayout(context);
		linear11.setOrientation(LinearLayout.VERTICAL);
		linear11.setLayoutParams(new LinearLayout.LayoutParams(
									 ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear11.setPadding(8, 8, 8, 8);
		linear2.addView(linear11);

		LinearLayout linear12 = new LinearLayout(context);
		linear12.setOrientation(LinearLayout.HORIZONTAL);
		linear12.setLayoutParams(new LinearLayout.LayoutParams(
									 ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		linear12.setPadding(8, 8, 8, 8);
		linear12.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					String url="mqqapi://card/show_pslcard?src_type=internal&version=1&uin=" + 三服配置区.QQ群号 + "&card_type=group&source=qrcode";
					startActivity(new android.content.Intent(android.content.Intent.ACTION_VIEW, android.net.Uri.parse(url)));
				}
			});
		linear11.addView(linear12);

		TextView textView13 = new TextView(context);
		textView13.setLayoutParams(new LinearLayout.LayoutParams(
									   ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
		textView13.setPadding(8, 8, 8, 8);
		textView13.setText("有什么问题加入官方QQ群");
		textView13.setTextSize(12);
		textView13.setTextColor(Color.BLACK);
		linear12.addView(textView13);

		TextView textView14 = new TextView(context);
		textView14.setLayoutParams(new LinearLayout.LayoutParams(
									   ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT));
		textView14.setPadding(8, 8, 8, 8);
		textView14.setText("加群");
		textView14.setTextSize(12);
		textView14.setTextColor(Color.parseColor("#F44336"));
		textView14.setGravity(Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL);
		linear12.addView(textView14);

		scrollView.addView(innerLayout);
		mainLayout.addView(scrollView);

		return mainLayout;
	}

}

