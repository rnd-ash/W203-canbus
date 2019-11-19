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

    val t = Thread(){
        Looper.prepare()
        while(true) {
            if (!isConnected) {
                btManager = BluetoothComm(device, false, adapter)
                openConnection()
            } else {
                Thread.sleep(5000L)
            }
        }
    }



    var btManager = BluetoothComm(device, false, adapter)
    private var isConnected = false

    fun sendBodyText(msg: String) = safeCommunication { btManager.sendString("B:$msg") }

    fun sendTrackName(name: String) = safeCommunication { btManager.sendString("M-$name") }

    fun sendHeaderText(msg: String) = safeCommunication{ btManager.sendString("H:$msg") }
    fun sendByteArray(id: Char, sep: Byte, bytes: ByteArray) = safeCommunication {
        btManager.sendBytes(byteArrayOf(id.toByte(), sep) + bytes)
    }
    fun setScrollSpeed(intervalMS: Int) = safeCommunication { btManager.sendString("S:$intervalMS") }
    fun setInidcatorSpeed(intervalMS: Int) = safeCommunication { btManager.sendString("A:$intervalMS") }

    private inline fun safeCommunication(x: () -> Unit) {
        if (isConnected) {
            try {
                x()
            } catch (e: IOException) {
                isConnected = false
            }
        }
    }


    fun toggleESP() = safeCommunication { btManager.sendString("C:1") }
    fun lockDoors() = safeCommunication { btManager.sendString("C:2") }
    fun unlockDoors() = safeCommunication { btManager.sendString("C:3") }
    fun retractHeadRest() = safeCommunication { btManager.sendString("C:4") }
    fun enableRightIndicator() = safeCommunication { btManager.sendString("C:5") }
    fun enableLeftIndicator() = safeCommunication { btManager.sendString("C:6") }
    fun enableHazards() = safeCommunication { btManager.sendString("C:7") }
    fun disableSignalLights() = safeCommunication { btManager.sendString("C:8") }

    fun openConnection() {
        try {
            btManager.connect()
            isConnected = true
            Toast.makeText(context, "Connected!", Toast.LENGTH_SHORT).show()
        } catch (e: Exception) {
            Toast.makeText(context, "Cannot connect to BT!", Toast.LENGTH_SHORT).show()
        }
        if (!t.isAlive) {
            t.start()
        }
    }

    fun closeConnection() {
        btManager.disconnect()
    }

    private fun randomID() : Int {
        return (0..128).random()
    }


    fun destroy() {
        if (isConnected) {
            btManager.disconnect()
        }
    }
}