package by.diplom.vibric_mobile_v2.utils

import com.hoho.android.usbserial.driver.FtdiSerialDriver
import com.hoho.android.usbserial.driver.ProbeTable
import com.hoho.android.usbserial.driver.UsbSerialProber

class CustomProber {
    companion object {
        fun getCustomProber(): UsbSerialProber {
            val customTable = ProbeTable()
            customTable.addProduct(0x483, 0x374B, FtdiSerialDriver::class.java)
            return UsbSerialProber(customTable)
        }
    }
}