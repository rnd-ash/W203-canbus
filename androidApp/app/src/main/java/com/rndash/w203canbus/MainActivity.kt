package com.rndash.w203canbus

import android.bluetooth.BluetoothAdapter
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.media.AudioManager
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.KeyEvent
import android.widget.*
import androidx.annotation.RequiresApi
import org.w3c.dom.Text

class MainActivity : AppCompatActivity() {
    companion object {
        lateinit var manager : AudioManager
        lateinit var ctx: Context
    }

    lateinit var textView: TextView
    lateinit var ic : CarCommunicator
    lateinit var artistCheck : CheckBox
    lateinit var albumCheck : CheckBox
    lateinit var thread : Thread


    @RequiresApi(Build.VERSION_CODES.O)
    override fun onCreate(savedInstanceState: Bundle?) {
        ctx = this.applicationContext
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val adapter = BluetoothAdapter.getDefaultAdapter()
        val dev = adapter.bondedDevices.first { it.name == "HC-06" }
        ic = CarCommunicator(dev, adapter, this.applicationContext)
        ic.openConnection()
        textView = findViewById(R.id.info)

        manager = this.getSystemService(Context.AUDIO_SERVICE) as AudioManager
        val playPauseBtn = findViewById<Button>(R.id.play)
        val nextTrackBtn = findViewById<Button>(R.id.next)
        val prevTrackBtn = findViewById<Button>(R.id.previous)
        val bluetoothBtn = findViewById<Button>(R.id.btTest)
        val textInput = findViewById<EditText>(R.id.customText)
        val speedBar = findViewById<SeekBar>(R.id.speed)
        val speedText = findViewById<TextView>(R.id.speedText)
        val lockDoors = findViewById<Button>(R.id.doorLock)
        val unlockDoors = findViewById<Button>(R.id.doorUnlock)
        val espButton = findViewById<Button>(R.id.esp)
        val headRests = findViewById<Button>(R.id.headrests)
        val hazards = findViewById<Button>(R.id.hazards)
        val rightindicator = findViewById<Button>(R.id.rind)
        val leftindicator = findViewById<Button>(R.id.lind)
        val signalsoff = findViewById<Button>(R.id.soff)
        val indicatorFreq = findViewById<SeekBar>(R.id.indicatorFreq)
        val indicatorFreqText = findViewById<TextView>(R.id.indicatorFreqText)
        artistCheck = findViewById<CheckBox>(R.id.artist)
        albumCheck = findViewById<CheckBox>(R.id.album)

        speedBar.min = 5
        speedBar.max = 100
        speedBar.progress = 30

        indicatorFreq.min = 10
        indicatorFreq.max = 200
        indicatorFreq.progress = 50

        lockDoors.setOnClickListener {
            ic.lockDoors()
        }

        unlockDoors.setOnClickListener {
            ic.unlockDoors()
        }

        espButton.setOnClickListener {
            ic.toggleESP()
        }

        headRests.setOnClickListener {
            ic.retractHeadRest()
        }

        hazards.setOnClickListener {
            ic.enableHazards()
        }

        leftindicator.setOnClickListener {
            ic.enableLeftIndicator()
        }

        rightindicator.setOnClickListener {
            ic.enableRightIndicator()
        }

        signalsoff.setOnClickListener {
            ic.disableSignalLights()
        }




        playPauseBtn.setOnClickListener {
            Log.i("BTN", "Play/Pause track pressed")
            val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE)
            manager.dispatchMediaKeyEvent(event)
        }

        class listener() : SeekBar.OnSeekBarChangeListener {
            private var ms: Int = 0
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                ms = progress
                speedText.text = "${progress*10} MS"
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                ic.setScrollSpeed(ms*10)
            }
        }

        class listenerFreq() : SeekBar.OnSeekBarChangeListener {
            private var ms: Int = 0
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                ms = progress
                ic.setInidcatorSpeed(progress*10)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                indicatorFreqText.text = "${ms*10} MS"
            }
        }

        indicatorFreq.setOnSeekBarChangeListener(listenerFreq())
        speedBar.setOnSeekBarChangeListener(listener())

        bluetoothBtn.setOnClickListener {
            ic.sendBodyText(textInput.text.toString())
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

        val iF = IntentFilter()
        iF.addAction("com.android.music.metachanged")
        iF.addAction("com.spotify.music.playbackstatechanged")
        iF.addAction("com.spotify.music.metadatachanged")
        registerReceiver(receiver, iF)
    }

    var trackName: String = ""
    var wasPlaying = false

    val receiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val intentAction = intent.action!!;

            if (intentAction.contains(".metadatachanged")) {
                println("meta changed")
                trackName = intent.getStringExtra("track")!!;
                val trackDuration = (intent.getIntExtra("length", 0) / 1000).toInt()
                Log.d("TK", "Track is $trackDuration seconds long")
                ic.sendTrackName(trackName);
                ic.sendByteArray('M',
                    0x20,
                    byteArrayOf(
                        (trackDuration/256).toInt().toByte(),
                        (trackDuration%256).toInt().toByte()
                    )
                )
            } else if (intentAction.contains(".playbackstatechanged")) {
                println("Play state changed!");
                val isPlaying = intent.getBooleanExtra("playing", true)
                val progress = (intent.getLongExtra("position", 0) / 1000).toInt()
                when(isPlaying) {
                    true -> {
                        ic.btManager.sendString("M:P")
                        //ic.btManager.sendString("M")
                    }
                    false -> {
                        ic.btManager.sendString("M:X")
                    }
                }
                textView.text = "Track: $trackName\nPlaying?: $isPlaying"
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        ic.destroy()
    }
}
