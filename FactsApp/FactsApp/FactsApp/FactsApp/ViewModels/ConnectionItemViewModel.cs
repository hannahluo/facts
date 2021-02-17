using System;
using System.Collections.Generic;
using System.Text;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;

namespace FactsApp.ViewModels
{
    public class ConnectionItemViewModel : BaseViewModel
    {
        public IDevice Device { get; private set; }
        public Guid Id => Device.Id;
        public bool IsConnected => Device.State == DeviceState.Connected;
        public int Rssi => Device.Rssi;
        public string Name => DisplayName();
        
        public ConnectionItemViewModel(IDevice device)
        {
            Device = device;
        }

        public void Update(IDevice newDevice = null)
        {
            if(newDevice != null)
            {
                Device = newDevice;
            }
            OnPropertyChanged(nameof(IsConnected));
            OnPropertyChanged(nameof(Rssi));
        }

        public string DisplayName()
        {
            if(Device.Name == null)
            {
                return "N/A";
            } else
            {
                return Device.Name;
            }
        }
    }
}
