package by.diplom.vibric_mobile_v2.ui.terminal

import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.Spannable
import android.text.SpannableStringBuilder
import android.text.method.ScrollingMovementMethod
import android.text.style.ForegroundColorSpan
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import by.diplom.vibric_mobile_v2.R
import by.diplom.vibric_mobile_v2.utils.UsbPermission
import by.diplom.vibric_mobile_v2.databinding.FragmentTerminalBinding
import by.diplom.vibric_mobile_v2.ui.utils.BundleSingleton
import com.hoho.android.usbserial.driver.UsbSerialPort
import com.hoho.android.usbserial.driver.UsbSerialProber
import com.hoho.android.usbserial.util.HexDump
import com.hoho.android.usbserial.util.SerialInputOutputManager
import java.io.IOException
import java.lang.UnsupportedOperationException
import java.util.EnumSet
import kotlin.Exception

class TerminalFragment : Fragment(), SerialInputOutputManager.Listener {
    private val INTENT_ACTION_GRANT_USB = "by.diplom.vibric_mobile_v2.GRANT_USB"
    private val WRITE_WAIT_MILLIS = 2000
    private val READ_WAIT_MILLIS = 2000
    private val STOP_CHAR = ')'.code.toByte()

    private var deviceId: Int = 0
    private var portNum: Int = 0
    private var baudRate: Int = 115200
    private var withIoManager = false
    private var deviceName: String = ""

    private lateinit var binding: FragmentTerminalBinding
    private lateinit var controlLines: ControlLines


    private var usbIoManager: SerialInputOutputManager? = null
    private var usbSerialPort: UsbSerialPort? = null
    private var usbPermission = UsbPermission.UNKNOWN
    private var connected = false

    private var receivedData: MutableList<Byte> = mutableListOf()

    private val broadCastReceiver: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            if (intent?.action == INTENT_ACTION_GRANT_USB) {
                usbPermission =
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false))
                        UsbPermission.GRANTED
                    else
                        UsbPermission.DENIED
                connect()
            }
        }
    }
    private val mainLooper = Handler(Looper.getMainLooper())



    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
            deviceId = it.getInt(getString(R.string.device_id_key))
            portNum = it.getInt(getString(R.string.port_number_key))
            baudRate = it.getInt(getString(R.string.baud_rate_key))
            withIoManager = it.getBoolean(getString(R.string.with_io_manager_key))
            deviceName = it.getString(getString(R.string.device_name_key)) ?: "Undefined"
        }
//        BundleSingleton.let {
//            deviceId = it.getInt(getString(R.string.device_id_key))
//            portNum = it.getInt(getString(R.string.port_number_key))
//            baudRate = it.getInt(getString(R.string.baud_rate_key))
//            withIoManager = it.getBoolean(getString(R.string.with_io_manager_key))
//        }
    }

    override fun onStart() {
        super.onStart()
        context?.registerReceiver(broadCastReceiver, IntentFilter(INTENT_ACTION_GRANT_USB))
    }

    override fun onStop() {
        context?.unregisterReceiver(broadCastReceiver)
        super.onStop()
    }

    override fun onResume() {
        super.onResume()
        if (!connected && (usbPermission == UsbPermission.UNKNOWN || usbPermission == UsbPermission.GRANTED))
            mainLooper.post(::connect)
    }

    override fun onPause() {
        if (connected) {
            status("disconnected")
            disconnect()
        }
        super.onPause()
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        binding = FragmentTerminalBinding.inflate(inflater, container, false)
//        val command = arguments?.getString(getString(R.string.command_key))
//        if (!command.isNullOrEmpty()) {
//            send(command)
//        }
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        binding.receiveText.movementMethod = ScrollingMovementMethod.getInstance()
//        val command = arguments?.getString(getString(R.string.command_key))
//        if (!command.isNullOrEmpty()) {
//            send(command)
//        }
        binding.deviceName.text = deviceName
        binding.sendBtn.setOnClickListener { send(binding.sendText.text.toString()) }
        controlLines = ControlLines(requireView()) // Мэйби тут view!!
        if (withIoManager) {
            binding.receiveBtn.visibility = View.GONE
        } else {
            binding.receiveBtn.setOnClickListener { read() }
        }
    }

    private fun connect() {
        val usbManager = requireActivity().getSystemService(Context.USB_SERVICE) as UsbManager
        var device: UsbDevice? = null
        for (v in usbManager.deviceList.values) {
            if (v.deviceId == deviceId) {
                device = v
                break
            }
        }
        if (device == null) {
            status("Connection failed: device not found")
            return
        }
        var driver = UsbSerialProber.getDefaultProber().probeDevice(device)
        if (driver == null) {
            // TODO Add custom prober
        }
        if (driver == null) {
            status("Connection failed: no driver for device")
            return
        }
        if (driver.ports.size < portNum) {
            status("Connnection failed: not enough ports at device")
            return
        }
        usbSerialPort = driver.ports[portNum]
        val usbConnection = usbManager.openDevice(driver.device)
        if (usbConnection == null && usbPermission == UsbPermission.UNKNOWN && !usbManager.hasPermission(driver.device)) {
            usbPermission = UsbPermission.REQUESTED
            val flags = PendingIntent.FLAG_MUTABLE // здесь возможно должны быть сравнения кодов версий
            val intent = Intent(INTENT_ACTION_GRANT_USB)
            intent.`package` = requireActivity().packageName
            val usbPermissionIntent = PendingIntent.getBroadcast(requireActivity(), 0, intent, flags)
            usbManager.requestPermission(driver.device, usbPermissionIntent)
            return
        }
        if (usbConnection == null) {
            if (!usbManager.hasPermission(driver.device))
                status("Connection failed: permission denied")
            else
                status("Conntection failed: open failed")
        }

        try {
            usbSerialPort?.open(usbConnection)
            try {
                usbSerialPort?.setParameters(baudRate, 8, 1, UsbSerialPort.PARITY_NONE)
            } catch (e: UnsupportedOperationException) {
                status("Unsupport setparameters")
            }
            if (withIoManager) {
                usbIoManager = SerialInputOutputManager(usbSerialPort, this)
                usbIoManager?.start()
            }
            status("Connected")
            connected = true
            controlLines.start()
        } catch (e: Exception) {
            status("Connection failed: ${e.message}")
            disconnect()
        }
    }

    private fun disconnect() {
        connected = false
        controlLines.stop()
        usbIoManager?.apply {
            listener = null
            stop()
        }
    }

    private fun send(str: String) {
        if (!connected) {
            Toast.makeText(activity, "not connected", Toast.LENGTH_SHORT).show()
            return
        }
        try {
            val data = (str).toByteArray()
            val spn = SpannableStringBuilder()
            spn.append("Send ${data.size} bytes\n")
            spn.append(HexDump.dumpHexString(data)).append("\n")
            spn.setSpan(ForegroundColorSpan(resources.getColor(R.color.terminal_send_text)), 0, spn.length, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE)
            binding.receiveText.append(spn)
            usbSerialPort?.write(data, WRITE_WAIT_MILLIS)
        } catch (e: Exception) {
            onRunError(e)
        }
    }

    private fun read() {
        if (!connected) {
            Toast.makeText(activity, "not connected", Toast.LENGTH_SHORT).show()
            return
        }
        try {
            val buffer = ByteArray(8192)
            val len = usbSerialPort?.read(buffer, READ_WAIT_MILLIS) ?: 0
            receive(buffer.copyOf(len))
        } catch (e: IOException) {
            status("Connection lost: ${e.message}")
            disconnect()
        }
    }

    private fun receive(data: ByteArray) {
        val spn = SpannableStringBuilder()
        spn.append("Receive ${data.size} bytes\n")
        if (data.isNotEmpty())
            spn.append(HexDump.dumpHexString(data)).append("\n")
            receivedData.addAll(data.asList())
        binding.receiveText.append(spn)
        if (data.any { it == STOP_CHAR }) {
            val span = SpannableStringBuilder()
            val resultData = receivedData.toByteArray().decodeToString()
            span.append("Response: $resultData\n")
            binding.receiveText.append(span)
            receivedData.clear()
        }
    }

    private fun status(str: String) {
        val spn = SpannableStringBuilder("$str\n")
        spn.setSpan(ForegroundColorSpan(resources.getColor(R.color.terminal_status_text)), 0, spn.length, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE)
        binding.receiveText.append(spn)
    }

    inner class ControlLines(private val view: View) {
        private val refreshInterval = 200
        private val runnable: Runnable = Runnable { run() }

        private fun run() {
            if (!connected)
                return
            try {
                val controlLines = usbSerialPort?.controlLines ?: EnumSet.noneOf(UsbSerialPort.ControlLine::class.java)
                mainLooper.postDelayed(runnable, refreshInterval.toLong())
            } catch (e: Exception) {
                status("getControlLines() failed: ${e.message} -> stopped control line refresh")
            }
        }

        fun start() {
           if (!connected)
               return
            try {
                val controlLines = usbSerialPort?.supportedControlLines ?: EnumSet.noneOf(UsbSerialPort.ControlLine::class.java)
                run()
            } catch (e: Exception) {
                Toast.makeText(activity, "getSupportedControlLines() failed: ${e.message}", Toast.LENGTH_SHORT).show()
            }
        }

        fun stop() {
            mainLooper.removeCallbacks(runnable)
        }
    }


    override fun onNewData(data: ByteArray?) {
        mainLooper.post { receive(data ?: byteArrayOf()) }
    }

    override fun onRunError(e: Exception?) {
        mainLooper.post {
            status("Connection lost: ${e?.message}")
            disconnect()
        }
    }

    companion object {
        fun newInstance(deviceId: Int, portNum: Int, baudRate: Int, withIoManager: Boolean) =
            TerminalFragment().apply {
                arguments = Bundle().apply {
                    putInt(getString(R.string.device_id_key), deviceId)
                    putInt(getString(R.string.port_number_key), portNum)
                    putInt(getString(R.string.baud_rate_key), baudRate)
                    putBoolean(getString(R.string.with_io_manager_key), withIoManager)
                }
            }
//            val fragment = TerminalFragment()
//            val args = Bundle()
//            val resources = fragment.requireContext().resources
//            args.putInt(resources.getString(R.string.device_id_key), deviceId)
//            args.putInt(resources.getString(R.string.port_number_key), portNum)
//            args.putInt(resources.getString(R.string.baud_rate_key), baudRate)
//            args.putBoolean(resources.getString(R.string.with_io_manager_key), withIoManager)
//            fragment.arguments = args
//            return fragment
    }
}

