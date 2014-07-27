package com.leaf.screenrecord_test_binder;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.support.v4.app.NavUtils;
import android.os.ServiceManager;
import android.os.IBinder;
import android.os.Parcel;
import android.widget.Button;
import android.widget.Toast;
import android.os.RemoteException;
import android.widget.EditText;
import android.widget.TextView;
public class ScreenRecord extends Activity {

    private Button btn1;
    private Button btn2;
    private IBinder binder;
    private EditText et1;
    private TextView tv1;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_screen_record);

		/*获取服务端的binder，在服务添加到sm的时候，sm为服务分配了一个handle，
		*获取服务就是获取这个handle,可以在bctest程序中找到证据。这里返回的binder是
		*new BpBinder(handle),返回的。
		*/
		binder = ServiceManager.getService("screenrecord");
		btn1 = (Button) findViewById(R.id.button1);
		btn2 = (Button) findViewById(R.id.button2);
		et1 = (EditText) findViewById(R.id.editText1);
		tv1 = (TextView) findViewById(R.id.textView1);
		if(binder != null && !binder.equals(null)){//可以将这个判断封装到transact
			checkSRState();

			btn1.setOnClickListener(new View.OnClickListener(){
			    public void onClick(View v) {
					Parcel w=Parcel.obtain();//获取一个Parcel对象，用于封装下发到内核的IPC数据
					Parcel reply=Parcel.obtain();//用于接收服务端返回的数据
					w.writeString("/sdcard/"+et1.getEditableText());
					try{
						/*将IPC数据发往服务，并取得返回值。binder中包含了服务的定位信息
						*数据包含命令：1 这里是开始录制屏幕，数据：/sdcard/*这个存储路径,
						＊如有需要可以向w中写入更多数据。
						*最后一个参数是flag，暂时使用默认值,不使用TF_ONEWAY，oneway是非异步无返回值
						*/
			    		boolean b = binder.transact(1,w,reply,0);
						if(!b)
							Toast.makeText(getApplicationContext(), "1 binder transact fail", Toast.LENGTH_SHORT).show();
						else {
							/*获取服务的对本次请求的返回值，可以有多个看服务程序如何写，
							*服务如返回A、B两个数据，那么就需要按照A、B的顺序取值，
							*/
							int iret=reply.readInt();
							if(iret < 0){
								Toast.makeText(getApplicationContext(), "1 binder transact return ="+ iret, Toast.LENGTH_SHORT).show();
							}else{
								tv1.setText("屏幕录制已经开始");
								moveTaskToBack (true);
							}
						}
					}catch(RemoteException e){
					}
				
				}
			});

			btn2.setOnClickListener(new View.OnClickListener(){
			    public void onClick(View v) {
					Parcel w=Parcel.obtain();
					Parcel reply=Parcel.obtain();
					try{
						//命令2 停止录制屏幕
			    		boolean b = binder.transact(2,w,reply,0);
						if(!b)
							Toast.makeText(getApplicationContext(), "2 binder transact fail", Toast.LENGTH_SHORT).show();
						else{
							int iret=reply.readInt();
							if(iret < 0){
								Toast.makeText(getApplicationContext(), "1 binder transact return ="+ iret, Toast.LENGTH_SHORT).show();
							}else
								tv1.setText("屏幕录制已结束");

						}
					}catch(RemoteException e){
					}
				
				}
			});
		}
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_screen_record, menu);
        return true;
    }
	public void checkSRState(){
		Parcel w=Parcel.obtain();
		Parcel reply=Parcel.obtain();
		try{
			//命令3 查询当前服务状态
    		boolean b = binder.transact(3,w,reply,0);
			if(!b)
				Toast.makeText(getApplicationContext(), "2 binder transact fail", Toast.LENGTH_SHORT).show();
			else{
				int istate=reply.readInt();
				int iret=reply.readInt();
				if(iret < 0){
					Toast.makeText(getApplicationContext(), "1 binder transact return ="+ iret, Toast.LENGTH_SHORT).show();
				}
				if(istate>0)
					tv1.setText("屏幕录制已经开始");

			}
		}catch(RemoteException e){
		}
		
	}

    
}
