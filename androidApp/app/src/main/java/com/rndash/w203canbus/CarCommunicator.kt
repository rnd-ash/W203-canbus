package com.rndash.w203canbus

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.content.Context
import android.content.Intent
import android.os.Looper
import android.util.Log
import android.view.KeyEvent
import android.widget.Toast
import androidx.core.content.ContextCompat.startActivity
import java.io.IOException
import java.lang.Exception

class CarCommunicator(private val device: BluetoothDevice, private val adapter: BluetoothAdapter, private val context: Context) {
    var shouldQuit = false
    companion object {
        fun nextSong() {
            val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_NEXT)
            MainActivity.manager.dispatchMediaKeyEvent(event)
        }

        fun previousSong() {
            val event = KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PREVIOUS)
            MainActivity.manager.dispatchMediaKeyEvent(event)
        }
    }

    var btManager = BluetoothComm(device, false, adapter)




    // ----- Communication functions to BT -----
    fun ping() =  btManager.sendPayload(BTPayload(BT_CMD_PING, byteArrayOf(0)));

    fun sendBodyText(msg: String) = safeCommunication {
        btManager.sendPayload(BTPayload(BT_CMD_TRACK_NAME, msg.toByteArray(Charsets.UTF_8)))
    }

    fun sendTrackName(name: String) = safeCommunication {
        btManager.sendPayload(BTPayload(BT_CMD_TRACK_NAME, name.toByteArray(Charsets.UTF_8)))
    }

    fun sendTrackDuration(durationSec: Int) = safeCommunication {
        btManager.sendPayload(BTPayload(BT_CMD_TRACK_LEN, byteArrayOf(
            (durationSec and 0xFF).toByte(),
            ((durationSec shr 8) and 0xFF).toByte()
        )))
    }

    fun sendTrackSeek(seekPos: Int) = safeCommunication {
        btManager.sendPayload(BTPayload(BT_CMD_TRACK_SEEK, byteArrayOf(
            (seekPos and 0xFF).toByte(),
            ((seekPos shr 8) and 0xFF).toByte()
        )))
    }

    fun sendPlayStatus(isPlaying: Boolean) = safeCommunication {
        val b: Byte = if (isPlaying) BT_PLAY else BT_PAUSE
        btManager.sendPayload(BTPayload(BT_CMD_MUSIC_CTRL, byteArrayOf(b)))
    }






    private inline fun safeCommunication(x: () -> Unit) {
        if (btManager.isConnected) {
            try {
                x()
            } catch (e: IOException) {
                Log.e("IC", "Oops. Skipping action")
            }
        } else {
            Log.e("IC", "Not connected. Skipping action")
        }
    }
}