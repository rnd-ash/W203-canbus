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

class MainActivity : AppCompatActivity() {
    lateinit var textView: TextView
    lateinit var ic : CarCommunicator
    lateinit var artistCheck : CheckBox
    lateinit var albumCheck : CheckBox
    lateinit var thread : Thread


    @RequiresApi(Build.VERSION_CODES.O)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val adapter = BluetoothAdapter.getDefaultAdapter()
        val dev = adapter.bondedDevices.first { it.name == "HC-06" }
        ic = CarCommunicator(dev, adapter, this.applicationContext)
        ic.openConnection()
        textView = findViewById(R.id.info)

        val manager = this.getSystemService(Context.AUDIO_SERVICE) as AudioManager
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
        artistCheck = findViewById<CheckBox>(R.id.artist)
        albumCheck = findViewById<CheckBox>(R.id.album)

        speedBar.min = 5
        speedBar.max = 100
        speedBar.progress = 30

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




        playPauseBtn.setOnClickListener {
            Log.i("BTN", "Play/Pause track pressed")
            val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE)
            manager.dispatchMediaKeyEvent(event)
        }

        class listener() : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                ic.setScrollSpeed(progress*10)
                speedText.text = "${progress*10} MS"
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {

            }
        }

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

        Thread() {
            while(true) {
                if (ic.btManager.readText.isNotEmpty()) {
                    if (ic.btManager.readText == "N") {
                        val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_NEXT)
                        manager.dispatchMediaKeyEvent(event)
                    } else if (ic.btManager.readText == "P") {
                        val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PREVIOUS)
                        manager.dispatchMediaKeyEvent(event)
                    }
                    ic.btManager.readText = ""
                }
                Thread.sleep(500L)
            }
        }.start()

        val iF = IntentFilter()
        iF.addAction("com.android.music.metachanged")
        iF.addAction("com.spotify.music.playbackstatechanged")
        iF.addAction("com.spotify.music.metadatachanged")
        registerReceiver(receiver, iF)
    }

    var lastTrack: String = ""
    var wasPlaying = false

    val receiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            Log.i("TRK", "Track modified!")
            val action = intent.action
            val isPlaying = intent.getBooleanExtra("playing", false)
            val player = intent.action
            val trackName = intent.getStringExtra("track")
            val albumName = intent.getStringExtra("album")
            val artistName = intent.getStringExtra("artist")
            if (trackName != lastTrack) {
                lastTrack = trackName
                var msg = trackName
                if (artistCheck.isChecked) {
                    msg += " by $artistName "
                }
                if (albumCheck.isChecked) {
                    msg += " in album $albumName"
                }
                ic.sendBodyText(msg)
            }
            textView.text = "Intent:$action\nTrack: $trackName\nArtist: $artistName\nAlbum: $albumName\nPlaying: $isPlaying"
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        ic.destroy()
    }
}
