package com.rndash.w203canbus

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.res.Configuration
import android.graphics.Color
import android.media.AudioManager
import android.media.MediaPlayer
import android.os.Build
import android.os.Bundle
import android.telephony.TelephonyManager
import android.util.Log
import android.view.KeyEvent
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import java.util.*
import kotlin.properties.Delegates


class MainActivity : AppCompatActivity() {
    companion object {
        lateinit var manager : AudioManager
        lateinit var ctx: Context
        var carrierName = "Unknown"
    }
    lateinit var thread : Thread
    var artistName = ""
    lateinit var service : ConnectService




    @RequiresApi(Build.VERSION_CODES.O)
    override fun onCreate(savedInstanceState: Bundle?) {
        carrierName = (this.getSystemService(Context.TELEPHONY_SERVICE) as TelephonyManager).networkOperatorName
        println(carrierName)
        var mPlayer = MediaPlayer.create(this, R.raw.xmas)
        val adapter = BluetoothAdapter.getDefaultAdapter()
        val dev = adapter.bondedDevices.first { it.name == "HC-06" }
        ConnectService.ic = CarCommunicator(dev, adapter, this.applicationContext)
        startService(Intent(this.baseContext, ConnectService::class.java))
        ctx = this.applicationContext
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        manager = this.getSystemService(Context.AUDIO_SERVICE) as AudioManager
        val playPauseBtn = findViewById<Button>(R.id.play)
        val nextTrackBtn = findViewById<Button>(R.id.next)
        val prevTrackBtn = findViewById<Button>(R.id.previous)
        val bluetoothBtn = findViewById<Button>(R.id.btTest)
        val textInput = findViewById<EditText>(R.id.customText)
        val statusText = findViewById<TextView>(R.id.arduino_status)
        val showBtn = findViewById<Button>(R.id.showtime)
        val hornBtn = findViewById<Button>(R.id.horn)
        val lightBtn = findViewById<Button>(R.id.lights)

        playPauseBtn.setOnClickListener {
            Log.i("BTN", "Play/Pause track pressed")
            val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE)
            manager.dispatchMediaKeyEvent(event)
        }

        bluetoothBtn.setOnClickListener {
            ConnectService.ic.sendBodyText(textInput.text.toString())
        }

        nextTrackBtn.setOnClickListener {
            Log.i("BTN", "Next track pressed")
            val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_NEXT)
            manager.dispatchMediaKeyEvent(event)
        }

        prevTrackBtn.setOnClickListener {
            Log.i("BTN", "Previous track pressed")
            val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PREVIOUS)
            manager.dispatchMediaKeyEvent(event)
        }

        showBtn.setOnClickListener {
            val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PAUSE)
            manager.dispatchMediaKeyEvent(event)
            mPlayer = MediaPlayer.create(this, R.raw.xmas)
            mPlayer.start()
            ConnectService.ic.sendByteArray('D',0x00, byteArrayOf())
        }

        lightBtn.setOnClickListener {
            mPlayer.stop()
            ConnectService.ic.sendByteArray('F',0x00, byteArrayOf())
        }

        hornBtn.setOnClickListener {
            mPlayer.stop()
            ConnectService.ic.sendByteArray('G',0x00, byteArrayOf())
        }

        val iF = IntentFilter()
        iF.addAction("com.android.music.metachanged")
        iF.addAction("com.spotify.music.playbackstatechanged")
        iF.addAction("com.spotify.music.metadatachanged")
        registerReceiver(receiver, iF)

        val timer = Timer()
        val t = object : TimerTask() {
            var off = false
            override fun run() {
                runOnUiThread {
                    when (ConnectService.conn_state) {
                        BT_STATE.CONNECTED -> {
                            statusText.text = "Status: Connected"
                            statusText.setTextColor(Color.GREEN)
                        }
                        BT_STATE.SCANNING, BT_STATE.DISCONNECTED -> {
                            when (off) {
                                true -> {
                                    statusText.text = " "
                                }
                                false -> {
                                    if (ConnectService.conn_state == BT_STATE.SCANNING) {
                                        statusText.text = "Status: Scanning"
                                        statusText.setTextColor(Color.parseColor("#ffa500"))
                                    } else {
                                        statusText.text = "Status: Disconnected"
                                        statusText.setTextColor(Color.RED)
                                    }
                                }
                            }
                            off = !off
                        }
                    }
                }
            }
        }
        timer.schedule(t, 0, 500)
    }

    var trackName: String by Delegates.observable("") { _, o, n ->
        if (o != n) {
            ConnectService.ic.sendTrackName(n);
        }
    }
    var wasPlaying = false

    val receiver = object : BroadcastReceiver() {
        @SuppressLint("SetTextI18n")
        override fun onReceive(context: Context, intent: Intent) {
            try {
                val intentAction = intent.action!!
                if (intentAction.contains(".metadatachanged")) {
                    println("meta changed")
                    trackName = intent.getStringExtra("track")!!
                    val trackDuration = (intent.getIntExtra("length", 0) / 1000).toInt()
                    Log.d("TK", "Track is $trackDuration seconds long");
                    ConnectService.ic.sendByteArray(
                        'M',
                        0x20,
                        byteArrayOf(
                            (trackDuration / 256).toInt().toByte(),
                            (trackDuration % 256).toInt().toByte()
                        )
                    )
                } else if (intentAction.contains(".playbackstatechanged")) {
                    val isPlaying = intent.getBooleanExtra("playing", true)
                    val progress = (intent.getLongExtra("position", 0) / 1000).toInt()
                    ConnectService.ic.sendByteArray(
                        'M',
                        0x5F, // underscore
                        byteArrayOf(
                            (progress / 256).toByte(),
                            (progress % 256).toByte()
                        )
                    )
                    when (isPlaying) {
                        true -> ConnectService.ic.btManager.sendString("M:P")
                        false -> ConnectService.ic.btManager.sendString("M:X")
                    }
                }
            } catch (e: UninitializedPropertyAccessException) {
                Log.d("IT", "IC not initialised")
            } catch (e: NullPointerException) {
            } catch (e:Exception) {
                e.printStackTrace()
            }
        }
    }

    override fun onPause() {
        super.onPause()

    }

    override fun onResume() {
        super.onResume()
    }

    override fun onStop() {
        super.onStop()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        println("Config changed")
    }

    override fun onDestroy() {
        super.onDestroy()
        unregisterReceiver(receiver)
        stopService(Intent(this.baseContext, ConnectService::class.java))
    }
}
