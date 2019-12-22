package com.rndash.w203canbus

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
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.KeyEvent
import android.widget.*
import androidx.annotation.RequiresApi
import org.w3c.dom.Text
import java.lang.Exception
import java.lang.NullPointerException
import java.util.*

class MainActivity : AppCompatActivity() {
    companion object {
        lateinit var manager : AudioManager
        lateinit var ctx: Context
    }

    lateinit var textView: TextView
    lateinit var artistCheck : CheckBox
    lateinit var thread : Thread
    var artistName = ""


    @RequiresApi(Build.VERSION_CODES.O)
    override fun onCreate(savedInstanceState: Bundle?) {
        var mPlayer = MediaPlayer.create(this, R.raw.xmas)
        val adapter = BluetoothAdapter.getDefaultAdapter()
        val dev = adapter.bondedDevices.first { it.name == "HC-06" }
        ConnectService.ic = CarCommunicator(dev, adapter, this.applicationContext)
        ctx = this.applicationContext
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        textView = findViewById(R.id.info)

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
        artistCheck = findViewById<CheckBox>(R.id.artist)

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
                    when (ConnectService.ic.btManager.isConnected) {
                        true -> {
                            statusText.setText("Status: Connected")
                            statusText.setTextColor(Color.GREEN)
                        }
                        false -> {
                            when (off) {
                                true -> {
                                    statusText.setText(" ")
                                }
                                false -> {
                                    when (ConnectService.ic.isScanning) {
                                        true -> {
                                            statusText.setText("Status: Scanning")
                                            statusText.setTextColor(Color.parseColor("#ffa500"))
                                        }
                                        false -> {
                                            statusText.setText("Status: Disconnected")
                                            statusText.setTextColor(Color.RED)
                                        }
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

    var trackName: String = ""
    var wasPlaying = false

    val receiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            try {
                val intentAction = intent.action!!
                if (intentAction.contains(".metadatachanged")) {
                    println("meta changed")
                    trackName = intent.getStringExtra("track")!!
                    val trackDuration = (intent.getIntExtra("length", 0) / 1000).toInt()
                    Log.d("TK", "Track is $trackDuration seconds long")
                    ConnectService.ic.sendTrackName(trackName);
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
                            (progress / 256).toInt().toByte(),
                            (progress % 256).toInt().toByte()
                        )
                    )
                    when (isPlaying) {
                        true -> {
                            ConnectService.ic.btManager.sendString("M:P")
                            //ic.btManager.sendString("M")
                        }
                        false -> {
                            ConnectService.ic.btManager.sendString("M:X")
                        }
                    }
                    textView.text = "Track: $trackName\nPlaying?: $isPlaying"
                }
            } catch (e: UninitializedPropertyAccessException) {
                Log.d("IT", "IC not initailised")
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
        ConnectService.ic.destroy()
        unregisterReceiver(receiver)
        stopService(Intent(this, ConnectService::class.java))
    }
}
