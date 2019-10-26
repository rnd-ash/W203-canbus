package com.rndash.w203canbus

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice

class CarCommunicator(device: BluetoothDevice, adapter: BluetoothAdapter) {
    private val btManager = BluetoothComm(device, false, adapter)

    fun sendBodyText(msg: String) {
        btManager.sendString("B:$msg")
    }

    fun sendHeaderText(msg: String) {
        btManager.sendString("H:$msg")
    }

    fun setScrollSpeed(intervalMS: Int) {
        btManager.sendString("S:$intervalMS")
    }

    fun toggleESP() = btManager.sendString("C:1")
    fun lockDoors() = btManager.sendString("C:2")
    fun unlockDoors() = btManager.sendString("C:3")
    fun retractHeadRest() = btManager.sendString("C:4")


    fun openConnection() {
        btManager.connect()
    }

    fun closeConnection() {
        btManager.disconnect()
    }

    private fun randomID() : Int {
        return (0..128).random()
    }

    fun getDebugOutput() {
        btManager.getData()
    }
}