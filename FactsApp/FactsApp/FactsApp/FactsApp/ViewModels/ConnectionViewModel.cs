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
using Acr.UserDialogs;

namespace FactsApp.ViewModels
{
    class ConnectionViewModel : BaseViewModel
    {
        readonly IPermissions m_permissions;
        IUserDialogs m_dialogs;

        
        public bool IsStateOn => m_ble.IsOn;
        public string StateText => GetStateText();
        
        public bool IsScanning => m_adapter.IsScanning;
        private bool _connectionEstablished = false;
        public bool ConnectionEstablished
        {
            get => _connectionEstablished;
            set
            {
                if (value != _connectionEstablished)
                {
                    _connectionEstablished = value;
                    OnPropertyChanged(nameof(ConnectionEstablished));
                }
            }
        }

        private Color _scanButtonColour = Color.Aqua;
        public Color ScanButtonColour
        {
            get => _scanButtonColour;
            set
            {
                if (value != _scanButtonColour)
                {
                    _scanButtonColour = value;
                    OnPropertyChanged(nameof(ScanButtonColour));
                }
            }
        }

        private string _scanButtonText = "Scan";
        public string ScanButtonText
        {
            get => _scanButtonText;
            set
            {
                if (value != _scanButtonText)
                {
                    _scanButtonText = value;
                    OnPropertyChanged(nameof(ScanButtonText));
                }
            }
        }
        private bool _scanButtonEnabled = true;
        public bool ScanButtonEnabled
        {
            get => _scanButtonEnabled;
            set
            {
                if (value != _scanButtonEnabled)
                {
                    _scanButtonEnabled = value;
                    OnPropertyChanged(nameof(ScanButtonEnabled));
                }
            }
        }

        private Color _disconnectButtonColour = Color.Red;
        public Color DisconnectButtonColour
        {
            get => _disconnectButtonColour;
            set
            {
                if(value != _disconnectButtonColour)
                {
                    _disconnectButtonColour = value;
                    OnPropertyChanged(nameof(DisconnectButtonColour));
                }
            }
        }
        private string _disconnectButtonText = "Disconnect";
        public string DisconnectButtonText
        {
            get => _disconnectButtonText;
            set
            {
                if(value != _disconnectButtonText)
                {
                    _disconnectButtonText = value;
                    OnPropertyChanged(nameof(DisconnectButtonText));
                }    
            }
        }
        private bool _disconnectButtonEnabled = true;
        public bool DisconnectButtonEnabled
        {
            get => _disconnectButtonEnabled;
            set
            {
                if(value != _disconnectButtonEnabled)
                {
                    _disconnectButtonEnabled = value;
                    OnPropertyChanged(nameof(DisconnectButtonEnabled));
                }
            }
        }
        private int _connDevIdx = -1;
        public ObservableCollection<ConnectionItemViewModel> Devices { get; set; } = new ObservableCollection<ConnectionItemViewModel>();


        public ConnectionViewModel(IUserDialogs dialog) : base()
        {
            
            m_permissions = CrossPermissions.Current;
            // Register event handlers for various events
            m_ble.StateChanged += OnStateChanged;
            m_adapter.DeviceDiscovered += (s, a) => Devices.Add(new ConnectionItemViewModel(a.Device));
            m_adapter.ScanTimeoutElapsed += FinishedScanning;
            m_adapter.DeviceConnected += OnConnection;
            m_adapter.DeviceDisconnected += OnDisconnection;
            m_adapter.DeviceConnectionLost += OnLostConnection;
            m_dialogs = dialog;
            ConnectionEstablished = false;
            _connDevIdx = -1;
            if (IsStateOn)
            {
                OnPropertyChanged(nameof(ScanButtonColour));
                ScanButtonEnabled = true;
                ScanButtonText = "Scan";
                ScanButtonColour = Color.Aqua;
            }
            else
            {
                OnPropertyChanged(nameof(ScanButtonColour));
                ScanButtonEnabled = false;
                ScanButtonText = "Bluetooth Off";
                ScanButtonColour = Color.LightGray;
            }
            DisconnectButtonColour = Color.Red;
            DisconnectButtonEnabled = false;
            DisconnectButtonText = "Disconnect";
        }

        private void OnConnection(object sender, EventArgs e)
        {
            ConnectionEstablished = true;
            OnPropertyChanged(nameof(ConnectionEstablished));
            DisconnectButtonEnabled = true;
            DisconnectButtonText = "Disconnect";
            DisconnectButtonColour = Color.Red;
        }
        private void FinishedScanning(object sender, EventArgs e)
        {
            ScanButtonEnabled = true;
            ScanButtonText = "Scan";
            ScanButtonColour = Color.Aqua;
        }

        public void OnStateChanged(object sender, BluetoothStateChangedArgs e)
        {
            if(IsStateOn)
            {
                ScanButtonEnabled = true;
                ScanButtonText = "Scan";
                ScanButtonColour = Color.Aqua;
            } else
            {
                ConnectionEstablished = false;
                ScanButtonEnabled = false;
                ScanButtonColour = Color.LightGray;
                ScanButtonText = "Bluetooth Off";
                _connDevIdx = -1;
                m_connectedDevice = null;
            }
            OnPropertyChanged(nameof(IsStateOn));
            OnPropertyChanged(nameof(StateText));
        }

        public void OnScanButtonClicked(object sender, EventArgs e)
        {
            // Initiate BLE scanning procedure
            if (!IsStateOn)
            {
                OnPropertyChanged(nameof(StateText));
                OnPropertyChanged(nameof(IsStateOn));
                m_dialogs.AlertAsync("Please turn on bluetooth");
                return;
            }

            // Get permissions on Android
            if (Xamarin.Forms.Device.RuntimePlatform == Device.Android)
            {
                CheckPermissions();
            }

            // Scan for available devices
            Scan();
        }

        private async void CheckPermissions()
        {
            var status = await m_permissions.CheckPermissionStatusAsync<LocationPermission>();
            if (status != PermissionStatus.Granted)
            {
                PermissionStatus permissionResult = await m_permissions.RequestPermissionAsync<LocationPermission>(); ;

                if (permissionResult != PermissionStatus.Granted)
                {
                    await m_dialogs.AlertAsync("App does not have correct permissions");
                    return;
                }
            }
            return;
        }

        private async void Scan()
        {
            if(IsStateOn && !IsScanning)
            {
                Devices.Clear();
                m_adapter.ScanMode = ScanMode.LowPower;
                ScanButtonEnabled = false;
                ScanButtonText = "Scanning";
                ScanButtonColour = Color.LightGray;
                await m_adapter.StartScanningForDevicesAsync();
            }
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

        public async void OnConnectionSelect(SelectedItemChangedEventArgs arg)
        {
            ConfirmConfig config = new ConfirmConfig()
            {
                Message = "Establish Connection?",
                OkText = "Connect",
                CancelText = "Cancel"
            };
            var result = await m_dialogs.ConfirmAsync(config);
            if(result == false)
            {
                return;
            }

            var connectResult = await Connect(Devices[arg.SelectedItemIndex]);
            if (connectResult == true)
            {
                _connDevIdx = arg.SelectedItemIndex;
                m_connectedDevice = Devices[_connDevIdx].Device;
            }
        }

        private async Task<bool> Connect(ConnectionItemViewModel device)
        {
            try
            {
                await m_adapter.ConnectToDeviceAsync(device.Device);
                await m_dialogs.AlertAsync("Connection succeeded!");
                return true;
            }
            catch (Exception ex)
            {
                await m_dialogs.AlertAsync(ex.Message, "Connection failed: ");
                return false;
            }
            finally
            {
                device.Update();
            }

        }

        private async void Disconnect(ConnectionItemViewModel device)
        {
            try
            {
                if (!device.IsConnected)
                    return;
                DisconnectButtonEnabled = false;
                DisconnectButtonText = "Disconnecting";
                DisconnectButtonColour = Color.LightGray;
                await m_adapter.DisconnectDeviceAsync(device.Device);
                return;
            }
            catch (Exception ex)
            {
                await m_dialogs.AlertAsync(ex.Message, "Disconnect error. Restart app");
                return;
            }
            finally
            {
                device.Update();
            }
        }

        public async void OnDisconnectButtonClicked(object sender, EventArgs e)
        {
            if(!ConnectionEstablished)
            {
                await m_dialogs.AlertAsync("Not Connected");
            }

            Disconnect(Devices[_connDevIdx]);
        }
        private void OnDisconnection(object sender, DeviceEventArgs e)
        {
            m_dialogs.AlertAsync("Disconnected!");
            DisconnectButtonEnabled = true;
            DisconnectButtonText = "Disconnected";
            DisconnectButtonColour = Color.Red;
            ConnectionEstablished = false;
            _connDevIdx = -1;
            m_connectedDevice = null;
        }
        private void OnLostConnection(object sender, DeviceErrorEventArgs e)
        {
            Devices.FirstOrDefault(d => d.Id == e.Device.Id)?.Update();
            m_dialogs.AlertAsync("Connection Lost!");
            ConnectionEstablished = false;
            DisconnectButtonEnabled = true;
            DisconnectButtonText = "Disconnected";
            DisconnectButtonColour = Color.Red;
            ScanButtonEnabled = true;
            ScanButtonText = "Scan";
            ScanButtonColour = Color.Aqua;
            _connDevIdx = -1;
            m_connectedDevice = null;
        }
    }
}
