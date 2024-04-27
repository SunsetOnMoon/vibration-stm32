package by.diplom.vibric_mobile_v2.ui.configuration

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
import android.text.style.ForegroundColorSpan
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.fragment.app.Fragment
import by.diplom.vibric_mobile_v2.R
import by.diplom.vibric_mobile_v2.databinding.FragmentConfigurationBinding
import by.diplom.vibric_mobile_v2.ui.utils.BundleSingleton
import by.diplom.vibric_mobile_v2.utils.UsbPermission
import by.diplom.vibric_mobile_v2.utils.getCurrentUtc
import com.hoho.android.usbserial.driver.UsbSerialPort
import com.hoho.android.usbserial.driver.UsbSerialProber
import com.hoho.android.usbserial.util.HexDump
import com.hoho.android.usbserial.util.SerialInputOutputManager
import java.io.IOException
import java.lang.UnsupportedOperationException

class ConfigurationFragment : Fragment(), SerialInputOutputManager.Listener {
    private val INTENT_ACTION_GRANT_USB = "by.diplom.vibric_mobile_v2.GRANT_USB"
    private val WRITE_WAIT_MILLIS = 2000
    private val READ_WAIT_MILLIS = 2000
    private val STOP_CHAR = ')'.code.toByte()

    private var deviceId: Int = 0
    private var portNum: Int = 0
    private var baudRate: Int = 115200
    private var withIoManager = false
    private var command: String = ""

    private lateinit var binding: FragmentConfigurationBinding


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
//        arguments?.let {
//            deviceId = it.getInt(getString(R.string.device_id_key))
//            portNum = it.getInt(getString(R.string.port_number_key))
//            baudRate = it.getInt(getString(R.string.baud_rate_key))
//            withIoManager = it.getBoolean(getString(R.string.with_io_manager_key))
//        }
        BundleSingleton.let {
            deviceId = it.getInt(getString(R.string.device_id_key))
            portNum = it.getInt(getString(R.string.port_number_key))
            baudRate = it.getInt(getString(R.string.baud_rate_key))
            withIoManager = it.getBoolean(getString(R.string.with_io_manager_key))
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        binding = FragmentConfigurationBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        val sampleSizes = listOf(256, 512, 1024, 2048, 4096, 8192, 16384, 32768)
        var sampleSize: Int = 0

        val adapter = ArrayAdapter(requireContext(), android.R.layout.simple_spinner_item, sampleSizes)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        binding.spinnerSample.adapter = adapter
        binding.spinnerSample.setOnItemClickListener { parent, view, position, id ->
            sampleSize = parent.getItemAtPosition(position) as Int
        }
        binding.spinnerSample

        binding.btnConfigure.setOnClickListener {
            if (binding.switchSynchronize.isChecked) {
                val currentTime = getCurrentUtc()
                val command = "(TIME,3,$currentTime)"
                send(command)
            }

            if (binding.teFreq.text.isNotEmpty() && sampleSize != 0) {
                val deltaF = binding.teFreq.text.toString().toFloat() //TODO сделай через try catch
                if (deltaF < 0.5) {
                    Toast.makeText(requireContext(), "Частоное разрешение должно быть больше 0.5 Гц", Toast.LENGTH_SHORT).show()
                }
                val analyseTime = 1 / deltaF;
                val freq: Int = (sampleSize / analyseTime).toInt()
                if (freq in 1000..25000) {
                    val counter = getFrequency(freq = freq)
                    val command = String.format("(SAMP,1,%012d)", counter)
                    send(command)
                } else {
                    Toast.makeText(requireContext(), "Частота должна быть от 1 кГц до 25 кГц", Toast.LENGTH_SHORT).show()
                }
            }
            else if ((sampleSize == 0) && (binding.teFreq.text.isNotEmpty())) {
                Toast.makeText(requireContext(), "Укажите размер выборки", Toast.LENGTH_SHORT).show()
            }
            else if ((sampleSize != 0) && (binding.teFreq.text.isEmpty())) {
                Toast.makeText(requireContext(), "Укажите частоное разрешение", Toast.LENGTH_SHORT).show()
            }

        }
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
//            status("disconnected")
            disconnect()
        }
        super.onPause()
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
//            status("Connected")
            connected = true
//            controlLines.start()
        } catch (e: Exception) {
            status("Connection failed: ${e.message}")
            disconnect()
        }
    }

    private fun disconnect() {
        connected = false
//        controlLines.stop()
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
//        binding.receiveText.append(spn)
        if (data.any { it == STOP_CHAR }) {
            val span = SpannableStringBuilder()
            val resultData = receivedData.toByteArray().decodeToString()
            span.append("Response: $resultData\n")
//            binding.receiveText.append(span)
            Toast.makeText(requireContext(), span, Toast.LENGTH_SHORT).show()
            receivedData.clear()
        }
    }

    private fun status(str: String) {
        val spn = SpannableStringBuilder("$str\n")
        spn.setSpan(ForegroundColorSpan(resources.getColor(R.color.terminal_status_text)), 0, spn.length, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE)
        Toast.makeText(requireContext(), spn, Toast.LENGTH_SHORT).show()
    }

    private fun getFrequency(sysFreq: Int = 64000000, prescaler: Int = 160, freq: Int): Int {
        return sysFreq / (prescaler * freq)
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
        fun newInstance(param1: String, param2: String) =
            ConfigurationFragment().apply {
                arguments = Bundle().apply {
                }
            }
    }
}