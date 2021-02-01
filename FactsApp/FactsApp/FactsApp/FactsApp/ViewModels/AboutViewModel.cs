using System;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;
using Xamarin.Forms.Shapes;

namespace FactsApp.ViewModels
{
    public class AboutViewModel : BaseViewModel
    {
        private double kneeModelThighLength = 50.0f;
        private double kneeModelBodyLength = 100.0f;
        private double kneeModelFootLength = 20.0f;

        private double flexionAngleValue = 0.0;
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

        private PointCollection dataPoints = new PointCollection();
        public PointCollection DataPoints
        {
            set
            {
                if (value != dataPoints)
                {
                    dataPoints = value;
                    OnPropertyChanged(nameof(DataPoints));
                }
            }
            get
            {
                return dataPoints;
            }
        }

        public AboutViewModel()
        {
            Title = "About";
            OpenWebCommand = new Command(async () => await Browser.OpenAsync("https://aka.ms/xamarin-quickstart"));

            var rand = new Random();

            Device.StartTimer(TimeSpan.FromSeconds(1), () =>
                {
                    PointCollection newPoints = new PointCollection();

                    int currWidth = (int)(Application.Current.MainPage.Width);
                    int currHeight = (int)(Application.Current.MainPage.Height);

                    FlexionAngleValue += 5;//= rand.Next(5, 15);
                    FlexionAngleValue = FlexionAngleValue % 185;

                    if (FlexionAngleValue > 10)
                    {
                        FlexionAngleColor = Color.Red;
                    }
                    else
                    {
                        FlexionAngleColor = Color.Green;
                    }

                    // Scale this based on the flexion angle to approximate
                    double kneeVerticalAngle = FlexionAngleValue * 45 / 180;
                    double hipHorizontalAngle = 90 + kneeVerticalAngle - FlexionAngleValue;
                    double bodyVerticalAngle = kneeVerticalAngle / 2;

                    double ankleX   = currWidth/2;
                    double ankleY   = kneeModelBodyLength + 2 * kneeModelThighLength + 10.0f;
                    double toeX     = ankleX - kneeModelFootLength;
                    double toeY     = ankleY;
                    double kneeY    = ankleY - Math.Cos(kneeVerticalAngle * Math.PI / 180) * kneeModelThighLength;
                    double kneeX    = ankleX - Math.Sin(kneeVerticalAngle * Math.PI / 180) * kneeModelThighLength;
                    double hipY     = kneeY - Math.Sin(hipHorizontalAngle * Math.PI / 180) * kneeModelThighLength;
                    double hipX     = kneeX + Math.Cos(hipHorizontalAngle * Math.PI / 180) * kneeModelThighLength;
                    double headY    = hipY - Math.Cos(bodyVerticalAngle * Math.PI / 180) * kneeModelBodyLength;
                    double headX    = hipX - Math.Sin(bodyVerticalAngle * Math.PI / 180) * kneeModelBodyLength;

                    newPoints.Add(new Point(toeX, toeY));
                    newPoints.Add(new Point(ankleX, ankleY));
                    newPoints.Add(new Point(kneeX, kneeY));
                    newPoints.Add(new Point(hipX, hipY));
                    newPoints.Add(new Point(headX, headY));

                    // Have to set this to notify an update in the UI
                    DataPoints = newPoints;

                    return true;
                });
        }

        public ICommand OpenWebCommand { get; }
    }
}