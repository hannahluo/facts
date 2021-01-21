using System;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace FactsApp.ViewModels
{
    public class AboutViewModel : BaseViewModel
    {
        double flexionAngleValue = 0.0;
        public double FlexionAngleValue
        {
            set
            {
                if (value != flexionAngleValue)
                {
                    flexionAngleValue = value;
                    OnPropertyChanged(nameof(FlexionAngleValue));
                }
            }
            get
            {
                return flexionAngleValue;
            }
        }

        private Color flexionAngleColor = Color.Green;
        public Color FlexionAngleColor
        {
            set
            {
                if (value != flexionAngleColor)
                {
                    flexionAngleColor = value;
                    OnPropertyChanged(nameof(FlexionAngleColor));
                }
            }
            get
            {
                return flexionAngleColor;
            }
        }
        public AboutViewModel()
        {
            Title = "About";
            OpenWebCommand = new Command(async () => await Browser.OpenAsync("https://aka.ms/xamarin-quickstart"));

            var rand = new Random();

            Device.StartTimer(TimeSpan.FromSeconds(1), () =>
                {
                    this.FlexionAngleValue = rand.Next(5, 15);

                    if (this.FlexionAngleValue > 10)
                    {
                        this.FlexionAngleColor = Color.Red;
                    }
                    return true;
                });
        }

        public ICommand OpenWebCommand { get; }
    }
}