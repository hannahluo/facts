using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel;

namespace FactsApp.Models
{
    public class JointAxis : INotifyPropertyChanged
    {
        public JointAxis(double x, double y, double z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        private double _x;
        public double X
        {
            get => _x;
            set
            {
                if (value != _x)
                {
                    _x = value;
                    OnPropertyChanged(nameof(X));
                }
            }
        }
        private double _y;
        public double Y
        {
            get => _y;
            set
            {
                if (value != _y)
                {
                    _y = value;
                    OnPropertyChanged(nameof(Y));
                }
            }
        }
        private double _z;
        public double Z
        {
            get => _z;
            set
            {
                if (value != _z)
                {
                    _z = value;
                    OnPropertyChanged(nameof(Z));
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string propertyName)
        {
            var changed = PropertyChanged;
            if (changed == null)
                return;

            changed.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
