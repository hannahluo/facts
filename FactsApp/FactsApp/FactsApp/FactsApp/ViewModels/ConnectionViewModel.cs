using System;
using System.Collections.Generic;
using System.Text;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Xamarin.Forms;
using Plugin.BLE;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;
using Plugin.BLE.Abstractions.Extensions;
using Plugin.Permissions.Abstractions;
using Plugin.Permissions;
using Plugin.Settings.Abstractions;

namespace FactsApp.ViewModels
{
    class ConnectionViewModel : BaseViewModel
    {
        readonly IPermissions m_permissions;

        private Color _buttonColour = Color.Aqua;
        public Color ButtonColour
        {
            get => _buttonColour;
            set
            {
                if (value != _buttonColour)
                {
                    _buttonColour = value;
                    OnPropertyChanged(nameof(ButtonColour));
                }
            }
        }
        public bool IsStateOn => m_ble.IsOn;
        public string StateText => GetStateText();
        private string _buttonText = "Scan";
        public string ButtonText
        {
            get => _buttonText;
            set
            {
                if (value != _buttonText)
                {
                    _buttonText = value;
                    OnPropertyChanged(nameof(ButtonText));
                }
            }
        }


        
        public ConnectionViewModel() : base()
        {
            m_ble.StateChanged += OnStateChanged;
            m_permissions = CrossPermissions.Current;
        }

        private void OnStateChanged(object sender, BluetoothStateChangedArgs e)
        {
            if(!IsStateOn)
            {
                ButtonColour = Color.Aqua;
            } else
            {
                ButtonColour = Color.Red;
            }
            OnPropertyChanged(nameof(IsStateOn));
            OnPropertyChanged(nameof(StateText));
        }

        public void OnButtonClicked(object sender, EventArgs e)
        {
            ButtonText = "Scanning";

            // Initiate BLE scanning procedure
            if (!m_ble.IsOn)
            {
                OnPropertyChanged(nameof(StateText));
                // Insert dialog box / pop up
            }

            // Get permissions on Android
            if (Xamarin.Forms.Device.RuntimePlatform == Device.Android)
            {
                CheckPermissions();
            }

        }

        private async void CheckPermissions()
        {
            var status = await m_permissions.CheckPermissionStatusAsync<LocationPermission>();
            if (status != PermissionStatus.Granted)
            {
                PermissionStatus permissionResult = await m_permissions.RequestPermissionAsync<LocationPermission>(); ;

                if (permissionResult != PermissionStatus.Granted)
                {
                    ButtonText = "Scan Error";
                    OnPropertyChanged(nameof(StateText));
                    // Have some sort of popup / UI Change
                    return;
                }
            }
            return;
        }

        private string GetStateText()
        {
            switch (m_ble.State)
            {
                case BluetoothState.Unknown:
                    return "Unknown BLE state.";
                case BluetoothState.Unavailable:
                    return "BLE is not available on this device.";
                case BluetoothState.Unauthorized:
                    return "You are not allowed to use BLE.";
                case BluetoothState.TurningOn:
                    return "BLE is warming up, please wait.";
                case BluetoothState.On:
                    return "BLE is on.";
                case BluetoothState.TurningOff:
                    return "BLE is turning off. That's sad!";
                case BluetoothState.Off:
                    return "BLE is off. Turn it on!";
                default:
                    return "Unknown BLE state.";
            }
        }
    }
}
