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
using Accord.Math;
using Accord.Math.Optimization;
using Accord.Math.Differentiation;

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
        private readonly int _numDataPoints = 100;
        private readonly int _waitLength = 100; // ms
        private RawGyro _rawGyroCalfReadings;
        private RawGyro _rawGyroThighReadings;

        private readonly Guid _calibServiceGuid = Guid.Parse("87C53994-8E33-4070-9131-8F56AA023E45");
        private readonly Guid _initCalCharGuid = Guid.Parse("87C53995-8E33-4070-9131-8F56AA023E45");
        private readonly Guid _calfJointAxisCharGuid = Guid.Parse("87C53996-8E33-4070-9131-8F56AA023E45");
        private readonly Guid _thighJointAxisCharGuid = Guid.Parse("87C53997-8E33-4070-9131-8F56AA023E45");

        private struct RawGyro
        {
            public RawGyro(int points)
            {
                X = new double[points];
                Y = new double[points];
                Z = new double[points];
            }
            public double[] X { get; set; }
            public double[] Y { get; set; }
            public double[] Z { get; set; }
        }

        private struct Vector
        {
            public Vector(double x, double y, double z)
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
            _rawGyroCalfReadings = new RawGyro(_numDataPoints);
            _rawGyroThighReadings = new RawGyro(_numDataPoints);
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
        private void ParseRawGyroMessage(ref RawGyro store, int idx, byte[] array)
        {
            store.X[idx] = BitConverter.ToDouble(array, 0);
            store.Y[idx] = BitConverter.ToDouble(array, 8);
            store.Z[idx] = BitConverter.ToDouble(array, 16);
            return;
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
                ParseRawGyroMessage(ref _rawGyroCalfReadings, i, calfMsg.Result);

                var thighMsg = Task.Delay(_waitLength).ContinueWith(_ =>
                                                        { return rawGyroThighChar.ReadAsync().Result; } );
                if(thighMsg == null || thighMsg.IsFaulted)
                {
                    return false;
                }
                ParseRawGyroMessage(ref _rawGyroThighReadings, i, thighMsg.Result);
            }
            return true;
        }

        private Vector JointAxisFromSph(double phi, double theta)
        {
            return new Vector(Math.Cos(phi) * Math.Cos(theta), Math.Cos(phi) * Math.Sin(theta), Math.Sin(phi));
        }

        private double Norm(ref Vector vec)
        {
            return Math.Sqrt(vec.X*vec.X + vec.Y*vec.Y + vec.Z*vec.Z);
        }

        private double CrossProductNorm(ref Vector a, ref Vector b)
        {
            Vector crossResult = new Vector();
            crossResult.X = a.Y * b.Z - a.Z * b.Y;
            crossResult.Y = a.Z * b.X - a.X * b.Z;
            crossResult.Z = a.X * b.Y - a.Y * b.X;
            return Norm(ref crossResult);
        }

        // 
        public async Task<bool> Calibrate(IUserDialogs dialog)
        {
            double[][] temp_inputs = Jagged.ColumnVector(new[] { 0.03, 0.1947, 0.425, 0.626, 1.253, 2.500, 3.740 });

            double[][] inputs = Jagged.CreateAs(new double[_numDataPoints, 6]);
            double[] outputs = new double[_numDataPoints];
            for(int i = 0; i < _numDataPoints; i++)
            {
                inputs[i][0] = _rawGyroThighReadings.X[i];
                inputs[i][1] = _rawGyroThighReadings.Y[i];
                inputs[i][2] = _rawGyroThighReadings.Z[i];
                inputs[i][3] = _rawGyroCalfReadings.X[i];
                inputs[i][4] = _rawGyroCalfReadings.Y[i];
                inputs[i][5] = _rawGyroCalfReadings.Z[i];
                outputs[i] = 0.0;
            }

            // 1 - thigh, 2 - calf
            // parameters: [phi1, phi2, theta1, theta2]
            LeastSquaresFunction function = (double[] parameters, double[] gyro) =>
            {
                Vector thighGyro = new Vector(gyro[0], gyro[1], gyro[2]);
                Vector calfGyro = new Vector(gyro[3], gyro[4], gyro[5]);
                Vector thighAxis = JointAxisFromSph(parameters[0], parameters[2]);
                Vector calfAxis = JointAxisFromSph(parameters[1], parameters[3]);

                return CrossProductNorm(ref thighGyro, ref thighAxis) - CrossProductNorm(ref calfGyro, ref calfAxis);
            };

            LeastSquaresGradientFunction gradient = (double[] parameters, double[] gyro, double[] result) =>
            {
                var phi_1 = parameters[0];
                var phi_2 = parameters[1];
                var theta_1 = parameters[2];
                var theta_2 = parameters[3];
                var g1x = gyro[0];
                var g1y = gyro[1];
                var g1z = gyro[2];
                var g2x = gyro[3];
                var g2y = gyro[4];
                var g2z = gyro[5];
                result[0] = (2 * (g1x * Math.Cos(phi_1) + g1z * Math.Cos(theta_1) * Math.Sin(phi_1)) * (g1x * Math.Sin(phi_1) - g1z * Math.Cos(phi_1) * Math.Cos(theta_1)) + 2 * (g1y * Math.Cos(phi_1) + g1z * Math.Sin(phi_1) * Math.Sin(theta_1)) * (g1y * Math.Sin(phi_1) - g1z * Math.Cos(phi_1) * Math.Sin(theta_1)) - 2 * Math.Cos(phi_1) * Math.Sin(phi_1) * Math.Pow(g1y * Math.Cos(theta_1) - g1x * Math.Sin(theta_1), 2)) / (2 * Math.Pow(Math.Abs(Math.Pow(Math.Cos(phi_1) * (g1y * Math.Cos(theta_1) - g1x * Math.Sin(theta_1)), 2)) + Math.Abs(Math.Pow(g1x * Math.Sin(phi_1) - g1z * Math.Cos(phi_1) * Math.Cos(theta_1), 2)) + Math.Abs(Math.Pow(g1y * Math.Sin(phi_1) - g1z * Math.Cos(phi_1) * Math.Sin(theta_1), 2)), 0.5));
                result[1] = -1*(2 * (g2x * Math.Cos(phi_2) + g2z * Math.Cos(theta_2) * Math.Sin(phi_2)) * (g2x * Math.Sin(phi_2) - g2z * Math.Cos(phi_2) * Math.Cos(theta_2)) + 2 * (g2y * Math.Cos(phi_2) + g2z * Math.Sin(phi_2) * Math.Sin(theta_2)) * (g2y * Math.Sin(phi_2) - g2z * Math.Cos(phi_2) * Math.Sin(theta_2)) - 2 * Math.Cos(phi_2) * Math.Sin(phi_2) * Math.Pow(g2y * Math.Cos(theta_2) - g2x * Math.Sin(theta_2), 2)) / (2 * Math.Pow(Math.Abs(Math.Pow(Math.Cos(phi_2) * (g2y * Math.Cos(theta_2) - g2x * Math.Sin(theta_2)), 2)) + Math.Abs(Math.Pow(g2x * Math.Sin(phi_2) - g2z * Math.Cos(phi_2) * Math.Cos(theta_2), 2)) + Math.Abs(Math.Pow(g2y * Math.Sin(phi_2) - g2z * Math.Cos(phi_2) * Math.Sin(theta_2), 2)), 0.5));
                result[2] = -(Math.Cos(phi_1) * (g1y * Math.Cos(theta_1) - g1x * Math.Sin(theta_1)) * (g1z * Math.Sin(phi_1) + g1x * Math.Cos(phi_1) * Math.Cos(theta_1) + g1y * Math.Cos(phi_1) * Math.Sin(theta_1))) / Math.Pow(Math.Abs(Math.Pow(Math.Cos(phi_1) * (g1y * Math.Cos(theta_1) - g1x * Math.Sin(theta_1)), 2)) + Math.Abs(Math.Pow(g1x * Math.Sin(phi_1) - g1z * Math.Cos(phi_1) * Math.Cos(theta_1), 2)) + Math.Abs(Math.Pow(g1y * Math.Sin(phi_1) - g1z * Math.Cos(phi_1) * Math.Sin(theta_1), 2)), 0.5);
                result[3] = (Math.Cos(phi_2) * (g2y * Math.Cos(theta_2) - g2x * Math.Sin(theta_2)) * (g2z * Math.Sin(phi_2) + g2x * Math.Cos(phi_2) * Math.Cos(theta_2) + g2y * Math.Cos(phi_2) * Math.Sin(theta_2))) / Math.Pow(Math.Abs(Math.Pow(Math.Cos(phi_2) * (g1y * Math.Cos(theta_2) - g2x * Math.Sin(theta_2)), 2)) + Math.Abs(Math.Pow(g2x * Math.Sin(phi_2) - g2z * Math.Cos(phi_2) * Math.Cos(theta_2), 2)) + Math.Abs(Math.Pow(g2y * Math.Sin(phi_2) - g2z * Math.Cos(phi_2) * Math.Sin(theta_2), 2)), 0.5);
            };

            var gn = new GaussNewton(parameters: 4)
            {
                Function = function,
                Gradient = gradient,
                Solution = new double[] { 0.0, 0.0, 0.0, 0.0 }
            };

            gn.MaxIterations = 20;


            gn.Minimize(inputs, outputs);
            if(!gn.HasConverged)
            {
                dialog.Alert("Calibration did not converge. Continuing");
            }
            Vector thigh = JointAxisFromSph(gn.Solution[0], gn.Solution[2]);
            Vector calf = JointAxisFromSph(gn.Solution[1], gn.Solution[3]);
            CalfX = calf.X;
            CalfY = calf.Y;
            CalfZ = calf.Z;
            ThighX = thigh.X;
            ThighY = thigh.Y;
            ThighZ = thigh.Z;

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
