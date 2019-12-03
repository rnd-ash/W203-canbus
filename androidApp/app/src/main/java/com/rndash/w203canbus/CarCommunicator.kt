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

        fun invokeAssistant() {
            Log.d("CC", "Invoking assistant")
            val intent = Intent()
            MainActivity.ctx.startActivity(Intent(Intent.ACTION_VOICE_COMMAND).setFlags(Intent.FLAG_ACTIVITY_NEW_TASK))
        }

        fun killAssistant() {
            Log.d("CC", "Killing assistant")

        }
    }

    var btManager = BluetoothComm(device, false, adapter)

    fun ping() =  btManager.sendString("-")
    fun sendBodyText(msg: String) = safeCommunication { btManager.sendString("B:$msg") }

    fun sendTrackName(name: String) = safeCommunication { btManager.sendString("M-$name") }
    fun sendHeaderText(msg: String) = safeCommunication{ btManager.sendString("H:$msg") }
    fun sendByteArray(id: Char, sep: Byte, bytes: ByteArray) = safeCommunication {
        btManager.sendBytes(byteArrayOf(id.toByte(), sep) + bytes)
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

    fun openConnection() {
        try {
            btManager.connect()
            Toast.makeText(context, "Connected!", Toast.LENGTH_SHORT).show()
        } catch (e: Exception) {
            Toast.makeText(context, "Cannot connect to BT!", Toast.LENGTH_SHORT).show()
        }
    }

    fun closeConnection() {
        btManager.disconnect()
    }

    private fun randomID() : Int {
        return (0..128).random()
    }

    /**
     * Thread to constantly check if Arduino is responding. If not, try and reconnect
     */
    var isScanning = false
    val t = Thread() {
        while(true) {
            var shouldReconnect = true
            if (btManager.isConnected) {
                Log.d("BT", "Pinging")
                try {
                    ping()
                    shouldReconnect = false
                    isScanning = false
                    context.startService(Intent(context, ConnectService::class.java).setAction("connect"))
                } catch (e: Exception) {
                    shouldReconnect = true
                    context.startService(Intent(context, ConnectService::class.java).setAction("disconnect"))
                    Log.e("BT", "Ping failed")
                }
            }
            if (shouldReconnect) {
                try {
                    isScanning = true
                    context.startService(Intent(context, ConnectService::class.java).setAction("scan"))
                    btManager.connect()
                } catch (e: Exception) {

                }
            }
            try {
                Thread.sleep(5000L)
            } catch (e: InterruptedException) {
                if(shouldQuit) {
                    break
                }
            }
        }
    }
    init {
        t.start()
    }

    // Called when app closes
    fun destroy() {
        if (btManager.isConnected) {
            btManager.sendString("!")
            Thread.sleep(1000L)
            btManager.disconnect()
        }
        shouldQuit = true
        t.interrupt()
    }
}