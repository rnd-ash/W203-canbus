package com.rndash.w203canbus

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.content.Context
import android.widget.Toast
import java.io.IOException
import java.lang.Exception

class CarCommunicator(device: BluetoothDevice, adapter: BluetoothAdapter, private val context: Context) {
    val btManager = BluetoothComm(device, false, adapter)
    private var isConnected = false

    fun sendBodyText(msg: String) = safeCommunication { btManager.sendString("B:$msg") }

    fun sendHeaderText(msg: String) = safeCommunication{ btManager.sendString("H:$msg") }

    fun setScrollSpeed(intervalMS: Int) = safeCommunication { btManager.sendString("S:$intervalMS") }

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


    fun openConnection() {
        try {
            btManager.connect()
            isConnected = true
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


    fun destroy() {
        if (isConnected) {
            btManager.disconnect()
        }
    }
}