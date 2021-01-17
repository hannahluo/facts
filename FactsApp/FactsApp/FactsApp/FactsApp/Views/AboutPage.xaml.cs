using System;
using System.ComponentModel;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace FactsApp.Views
{
    public partial class AboutPage : ContentPage
    {
        public AboutPage()
        {
            InitializeComponent();
            BindingContext = this;
        }

        private double _flexionAngleValue = 0.0;
        public double FlexionAngleValue
        {
            get => _flexionAngleValue;
            set
            {
                if (value != _flexionAngleValue)
                {
                    _flexionAngleValue = value;
                    OnPropertyChanged(nameof(FlexionAngleValue));
                }
            }
        }

        private Color _flexionAngleColor = Color.Green;
        public Color FlexionAngleColor
        {
            get => _flexionAngleColor;
            set
            {
                if (value != _flexionAngleColor)
                {
                    _flexionAngleColor = value;
                    OnPropertyChanged(nameof(FlexionAngleColor));
                }
            }
        }
    }
}