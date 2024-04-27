package by.diplom.vibric_mobile_v2

import android.os.Bundle
import android.view.View
import com.google.android.material.bottomnavigation.BottomNavigationView
import androidx.appcompat.app.AppCompatActivity
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import by.diplom.vibric_mobile_v2.databinding.ActivityMainBinding
import by.diplom.vibric_mobile_v2.ui.utils.BundleSingleton

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val navView: BottomNavigationView = binding.navView

        val navController = findNavController(R.id.nav_host_fragment_activity_main)
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        val appBarConfiguration = AppBarConfiguration(
            setOf(
                R.id.navigation_terminal, R.id.navigation_configuration, R.id.navigation_settings
            )
        )
        setupActionBarWithNavController(navController, appBarConfiguration)
        navView.setupWithNavController(navController)
    }

    fun hideBottomNavigationView() {
        binding.navView.visibility = View.GONE
    }

    fun showBottomNavigationView() {
        binding.navView.visibility = View.VISIBLE
    }

    fun switchToTerminalFragmentAndSendCommand(command: String) {
        val navController = findNavController(R.id.nav_host_fragment_activity_main)
        navController.navigate(R.id.navigation_terminal)

        BundleSingleton.apply {
            putString(getString(R.string.command_key), command)
        }
    }
}