using System;
using System.Collections.Generic;
using System.Text;
using Xamarin.Forms;
using FactsApp.Models;
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
    class CalibrationViewModel : BaseViewModel
    {
        IUserDialogs m_dialogs;

        private JointAxis _thighAxis = new JointAxis(0,0,0);
        public JointAxis ThighAxis { get; set; }
        private JointAxis _calfAxis = new JointAxis(0,0,0);
        public JointAxis CalfAxis { get; set; }

        private bool _resultVisible = false;
        public bool ResultVisible
        {
            get => _resultVisible;
            set
            {
                if(value != _resultVisible)
                {
                    _resultVisible = value;
                    OnPropertyChanged(nameof(ResultVisible));
                }
            }
        }

        private Color _calibrateButtonColour = Color.Aqua;
        public Color CalibrateButtonColour
        {
            get => _calibrateButtonColour;
            set
            {
                if (value != _calibrateButtonColour)
                {
                    _calibrateButtonColour = value;
                    OnPropertyChanged(nameof(CalibrateButtonColour));
                }
            }
        }

        private string _calibrateButtonText = "Calibrate";
        public string CalibrateButtonText
        {
            get => _calibrateButtonText;
            set
            {
                if (value != _calibrateButtonText)
                {
                    _calibrateButtonText = value;
                    OnPropertyChanged(nameof(CalibrateButtonText));
                }
            }
        }
        private bool _calibrateButtonEnabled = true;
        public bool CalibrateButtonEnabled
        {
            get => _calibrateButtonEnabled;
            set
            {
                if (value != _calibrateButtonEnabled)
                {
                    _calibrateButtonEnabled = value;
                    OnPropertyChanged(nameof(CalibrateButtonEnabled));
                }
            }
        }

        private string _directionLabelText = "Please walk around and move both thigh and calf for 15s";
        public string DirectionLabelText
        {
            get => _directionLabelText;
            set
            {
                if(value != _directionLabelText)
                {
                    _directionLabelText = value;
                    OnPropertyChanged(nameof(DirectionLabelText));
                }
            }
        }

        public CalibrationViewModel(IUserDialogs dialog) : base()
        {
            m_dialogs = dialog;
            CalibrateButtonColour = Color.Aqua;
            CalibrateButtonText = "Calibrate";
            CalibrateButtonEnabled = true;
            ThighAxis = new JointAxis(0, 0, 0);
            CalfAxis = new JointAxis(0, 0, 0);
            ResultVisible = false;
        }

        public void OnCalibrateButtonClicked(object sender, EventArgs e)
        {
            if(m_connectedDevice == null)
            {
                m_dialogs.Alert("Not connected to any device!");
                return;
            }

            CalibrateButtonText = "Calibrating";
            CalibrateButtonEnabled = false;
            CalibrateButtonColour = Color.LightGray;

            // Start collecting calibration data from device

            // Display Results
            m_dialogs.Alert("Calibration Successful!");
            CalibrateButtonText = "Calibrate";
            CalibrateButtonEnabled = true;
            CalibrateButtonColour = Color.Aqua;
            ResultVisible = true;
            CalfAxis.X++;
            CalfAxis.Y++;
            CalfAxis.Z++;
            ThighAxis.X++;
            ThighAxis.Y++;
            ThighAxis.Z++;
            
        }

        public void OnAppearing()
        {
            if(m_connectedDevice == null)
            {
                CalibrateButtonColour = Color.LightGray;
                CalibrateButtonText = "Not Connected";
                CalibrateButtonEnabled = false;
            } 
            else
            {
                CalibrateButtonColour = Color.Aqua;
                CalibrateButtonText = "Calibrate";
                CalibrateButtonEnabled = true;
            }
        }
    }
}
