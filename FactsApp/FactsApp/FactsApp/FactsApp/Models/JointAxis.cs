using System;
using System.Collections.Generic;
using System.Text;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading;
using System.ComponentModel;
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

namespace FactsApp.Models
{
    public class JointAxis : INotifyPropertyChanged
    {
        private double _tX;
        public double ThighX
        {
            get => _tX;
            set
            {
                if (value != _tX)
                {
                    _tX = value;
                    OnPropertyChanged(nameof(ThighX));
                }
            }
        }
        private double _tY;
        public double ThighY
        {
            get => _tY;
            set
            {
                if (value != _tY)
                {
                    _tY = value;
                    OnPropertyChanged(nameof(ThighY));
                }
            }
        }
        private double _tZ;
        public double ThighZ
        {
            get => _tZ;
            set
            {
                if (value != _tZ)
                {
                    _tZ = value;
                    OnPropertyChanged(nameof(ThighZ));
                }
            }
        }

        private double _cX;
        public double CalfX
        {
            get => _cX;
            set
            {
                if (value != _cX)
                {
                    _cX = value;
                    OnPropertyChanged(nameof(CalfX));
                }
            }
        }
        private double _cY;
        public double CalfY
        {
            get => _cY;
            set
            {
                if (value != _cY)
                {
                    _cY = value;
                    OnPropertyChanged(nameof(CalfY));
                }
            }
        }
        private double _cZ;
        public double CalfZ
        {
            get => _cZ;
            set
            {
                if (value != _cZ)
                {
                    _cZ = value;
                    OnPropertyChanged(nameof(CalfZ));
                }
            }
        }

        private readonly Guid _imuRawServiceGuid = Guid.Parse("87C539A0-8E33-4070-9131-8F56AA023E45");
        private readonly Guid _rawGyroCalfCharGuid = Guid.Parse("87C539A1-8E33-4070-9131-8F56AA023E45");
        private readonly Guid _rawGyroThighCharGuid = Guid.Parse("87C539A2-8E33-4070-9131-8F56AA023E45");
        private readonly int _numDataPoints = 10;
        private readonly int _waitLength = 1000; // ms
        private RawGyro[] _rawGyroCalfReadings;
        private RawGyro[] _rawGyroThighReadings;

        private readonly Guid _calibServiceGuid = Guid.Parse("87C53994-8E33-4070-9131-8F56AA023E45");
        private readonly Guid _initCalCharGuid = Guid.Parse("87C53995-8E33-4070-9131-8F56AA023E45");
        private readonly Guid _calfJointAxisCharGuid = Guid.Parse("87C53996-8E33-4070-9131-8F56AA023E45");
        private readonly Guid _thighJointAxisCharGuid = Guid.Parse("87C53997-8E33-4070-9131-8F56AA023E45");

        private struct RawGyro
        {
            public RawGyro(double x, double y, double z)
            {
                X = x;
                Y = y;
                Z = z;
            }
            public double X { get; set; }
            public double Y { get; set; }
            public double Z { get; set; }
        }

        public JointAxis()
        {
            ThighX = 0;
            ThighY = 0;
            ThighZ = 0;
            CalfX = 0;
            CalfY = 0;
            CalfZ = 0;
            _rawGyroCalfReadings = new RawGyro[_numDataPoints];
            _rawGyroThighReadings = new RawGyro[_numDataPoints];
        }

        

        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string propertyName)
        {
            var changed = PropertyChanged;
            if (changed == null)
                return;

            changed.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        // Parse a raw gyro message
        private RawGyro ParseRawGyroMessage(byte[] array)
        {
            double x = BitConverter.ToDouble(array, 0);
            double y = BitConverter.ToDouble(array, 8);
            double z = BitConverter.ToDouble(array, 16);
            return new RawGyro(x, y, z);
        }

        // Get raw gyro data from device
        public async Task<bool> GetRawData(IDevice device, IUserDialogs dialog)
        {
            var calService = await device.GetServiceAsync(_imuRawServiceGuid);
            var rawGyroThighChar = await calService.GetCharacteristicAsync(_rawGyroThighCharGuid);
            var rawGyroCalfChar = await calService.GetCharacteristicAsync(_rawGyroCalfCharGuid);
            if(rawGyroCalfChar == null || rawGyroThighChar == null)
            {
                dialog.Alert("Could not find corresponding characteristics");
                return false;
            }
            if(!rawGyroCalfChar.CanRead || !rawGyroThighChar.CanRead)
            {
                dialog.Alert("Characteristics do not have the correct read permissions");
                return false;
            }

            for(int i = 0; i < _numDataPoints; i++)
            {
                var calfMsg = Task.Delay(_waitLength).ContinueWith(_ =>
                                                     { return rawGyroCalfChar.ReadAsync().Result; } );
                if(calfMsg == null || calfMsg.IsFaulted)
                {
                    return false;
                }
                _rawGyroCalfReadings[i] = ParseRawGyroMessage(calfMsg.Result);
                CalfX = _rawGyroCalfReadings[i].X;
                CalfY = _rawGyroCalfReadings[i].Y;
                CalfZ = _rawGyroCalfReadings[i].Z;

                var thighMsg = Task.Delay(_waitLength).ContinueWith(_ =>
                                                        { return rawGyroThighChar.ReadAsync().Result; } );
                if(thighMsg == null || calfMsg.IsFaulted)
                {
                    return false;
                }
                _rawGyroThighReadings[i] = ParseRawGyroMessage(thighMsg.Result);
                ThighX = _rawGyroThighReadings[i].X;
                ThighY = _rawGyroThighReadings[i].Y;
                ThighZ = _rawGyroThighReadings[i].Z;
            }
            return true;
        }

        // Perform calibration routine
        public async Task<bool> Calibrate()
        {
            CalfX = _rawGyroCalfReadings[9].X;
            CalfY = _rawGyroCalfReadings[9].Y;
            CalfZ = _rawGyroCalfReadings[9].Z;

            ThighX = _rawGyroThighReadings[9].X;
            ThighY = _rawGyroThighReadings[9].Y;
            ThighZ = _rawGyroThighReadings[9].Z;
            return true;
        }

        // Send results
        public async Task<bool> SendResult(IDevice device, IUserDialogs dialog)
        {
            var calibService = await device.GetServiceAsync(_calibServiceGuid);
            var calfJointAxisChar = await calibService.GetCharacteristicAsync(_calfJointAxisCharGuid);
            var thighJointAxisChar = await calibService.GetCharacteristicAsync(_thighJointAxisCharGuid);
            if (calfJointAxisChar == null || thighJointAxisChar == null)
            {
                dialog.Alert("Could not find corresponding characteristics");
                return false;
            }
            if (!calfJointAxisChar.CanWrite || !thighJointAxisChar.CanWrite)
            {
                dialog.Alert("Characteristics do not have the correct write permissions");
                return false;
            }

            var sendBuff = new byte[sizeof(double) * 3];

            BitConverter.GetBytes(CalfX).CopyTo(sendBuff, 0);
            BitConverter.GetBytes(CalfY).CopyTo(sendBuff, 8);
            BitConverter.GetBytes(CalfZ).CopyTo(sendBuff, 16);
            await calfJointAxisChar.WriteAsync(sendBuff);
            
            BitConverter.GetBytes(ThighX).CopyTo(sendBuff, 0);
            BitConverter.GetBytes(ThighY).CopyTo(sendBuff, 8);
            BitConverter.GetBytes(ThighZ).CopyTo(sendBuff, 16);
            await thighJointAxisChar.WriteAsync(sendBuff);

            return true;
        }
    }
}
