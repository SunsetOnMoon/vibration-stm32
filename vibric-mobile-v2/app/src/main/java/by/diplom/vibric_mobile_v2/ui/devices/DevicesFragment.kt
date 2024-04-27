package by.diplom.vibric_mobile_v2.ui.devices

import android.content.Context
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import by.diplom.vibric_mobile_v2.MainActivity
import by.diplom.vibric_mobile_v2.R
import by.diplom.vibric_mobile_v2.utils.CustomProber
import by.diplom.vibric_mobile_v2.databinding.FragmentDevicesBinding
import by.diplom.vibric_mobile_v2.databinding.ListItemDeviceBinding
import by.diplom.vibric_mobile_v2.ui.utils.BundleSingleton
import com.hoho.android.usbserial.driver.UsbSerialDriver
import com.hoho.android.usbserial.driver.UsbSerialProber
import java.util.Locale

class DevicesFragment : Fragment() {
    data class ListItem(
        val device: UsbDevice,
        val port: Int,
        val driver: UsbSerialDriver?
    )

    private val listItems = ArrayList<ListItem>()
    private lateinit var listAdapter: DeviceListAdapter
    private var baudRate = 115200
    private var withIoManager = true

    private val deviceFragmentBinding by lazy {
        FragmentDevicesBinding.inflate(layoutInflater)
    }

    private val deviceListItemBinding by lazy {
        ListItemDeviceBinding.inflate(layoutInflater)
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        return deviceFragmentBinding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        listAdapter = DeviceListAdapter(requireContext(), listItems) { item ->
            onListItemClick(item)
        }

        deviceFragmentBinding.rvDevices.apply {
            layoutManager = LinearLayoutManager(requireContext())
            adapter = listAdapter
        }
        refresh()
        deviceFragmentBinding.btnNext.setOnClickListener {
            if (listItems.isNotEmpty()) {

            }
        }
    }

    override fun onResume() {
        super.onResume()
        if (activity is MainActivity) {
            (activity as MainActivity).hideBottomNavigationView()
        }
    }

    override fun onStop() {
        super.onStop()
        if (activity is MainActivity) {
            (activity as MainActivity).showBottomNavigationView()
        }
    }
    private fun refresh() {
        val usbManager = requireActivity().getSystemService(Context.USB_SERVICE) as UsbManager
        val usbDefaultProber = UsbSerialProber.getDefaultProber()
        val usbCustomProber = CustomProber.getCustomProber()
        listItems.clear()
        for (device in usbManager.deviceList.values) {
            var driver = usbDefaultProber.probeDevice(device)
            if (driver == null)
                driver = usbCustomProber.probeDevice(device)
            if (driver != null) {
                for (port in 0 until driver.ports.size)
//                    if (device.)
                    listItems.add(ListItem(device, port, driver))
            } else {
                listItems.add(ListItem(device, 0, null))
            }
        }
        listAdapter.notifyDataSetChanged()
    }

    private fun onListItemClick(item: ListItem) {
        if (item.driver == null) {
            Toast.makeText(requireContext(), "no driver", Toast.LENGTH_SHORT).show()
        } else {
            val args = Bundle().apply {
                putInt(getString(R.string.device_id_key), item.device.deviceId)
                putInt(getString(R.string.port_number_key), item.port)
                putInt(getString(R.string.baud_rate_key), baudRate)
                putBoolean(getString(R.string.with_io_manager_key), withIoManager)
                putString(getString(R.string.device_name_key), item.device.productName)
            }
            BundleSingleton.apply {
                putInt(getString(R.string.device_id_key), item.device.deviceId)
                putInt(getString(R.string.port_number_key), item.port)
                putInt(getString(R.string.baud_rate_key), baudRate)
                putBoolean(getString(R.string.with_io_manager_key), withIoManager)
            }
//            val bottomNavigationView = requireView().findViewById<BottomNavigationView>(R.id.nav_view)
//            bottomNavigationView.visibility = View.VISIBLE
            findNavController().navigate(
                R.id.action_devicesFragment_to_navigation_terminal,
                args
//                BundleSingleton.getBundle()
            )
        }
    }

    companion object {
        fun newInstance(param1: String, param2: String) =
            DevicesFragment().apply {
                arguments = Bundle().apply {
                }
            }
    }
}

class DeviceListAdapter(
    private val context: Context,
    private val items: List<DevicesFragment.ListItem>,
    private val onItemClick: (DevicesFragment.ListItem) -> Unit
) : RecyclerView.Adapter<DeviceListAdapter.ViewHolder>() {

    override fun onCreateViewHolder(
        parent: ViewGroup,
        viewType: Int
    ): DeviceListAdapter.ViewHolder {
        val binding = ListItemDeviceBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return ViewHolder(binding)
    }

    override fun onBindViewHolder(holder: DeviceListAdapter.ViewHolder, position: Int) {
        val item = items[position]
        holder.bind(item)
    }

    override fun getItemCount(): Int = items.size

    inner class ViewHolder(private val binding: ListItemDeviceBinding) : RecyclerView.ViewHolder(binding.root) {
        init {
            binding.root.setOnClickListener {
                onItemClick.invoke(items[adapterPosition])
            }
        }

        fun bind(item: DevicesFragment.ListItem) {
            with(binding) {
                if (item.driver == null)
                    text1.text = "<no driver>"
                else if (item.driver.ports.size == 1) {
                    text1.text = item.driver.javaClass.simpleName.replace("SerialDriver", "")
                    text2.text = "No devices found"
                    imgMc.visibility = View.GONE
                }
                else {
                    text1.text = item.driver.javaClass.simpleName.replace(
                        "SerialDriver",
                        ""
                    ) + ", Port " + item.port
                    text2.text = "No devices found"
                    imgMc.visibility = View.GONE
                }
                text2.text = String.format(
                    Locale.US,
                    "Product name: %S, Vendor %04X, Product %04X",
                    item.device.productName,
                    item.device.vendorId,
                    item.device.productId
                )
            }
        }
    }
}