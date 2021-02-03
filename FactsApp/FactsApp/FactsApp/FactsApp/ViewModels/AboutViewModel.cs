using System;
using System.Collections.Generic;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;
using Xamarin.Forms.Shapes;
using Microcharts;
using SkiaSharp;

namespace FactsApp.ViewModels
{
    public class AboutViewModel : BaseViewModel
    {
        private double kneeModelThighLength = 50.0f;
        private double kneeModelBodyLength = 100.0f;
        private double kneeModelFootLength = 20.0f;

        private int maxRecentAngles = 20;
        private List<ChartEntry> recentAngleValues = new List<ChartEntry>();

        private double flexionAngleValue = 90.0;
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

        private Chart recentAngleChart;
        public Chart RecentAngleChart
        {
            set
            {
                if (value != recentAngleChart)
                {
                    recentAngleChart = value;
                    OnPropertyChanged(nameof(RecentAngleChart));
                }
            }
            get
            {
                return recentAngleChart;
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

                    FlexionAngleValue += rand.Next(-15, 15);

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

                    // Store most recent set of angles
                    recentAngleValues.Insert(0, new ChartEntry((float)flexionAngleValue));

                    if (recentAngleValues.Count > maxRecentAngles)
                    {
                        recentAngleValues.RemoveAt(maxRecentAngles);
                    }

                    RecentAngleChart = new LineChart()
                    {
                        Entries = recentAngleValues,
                        IsAnimated = false,
                        AnimationDuration = new TimeSpan(0), // Add this to prevent the chart from redrawing the intro animation every single time
                        MaxValue = 180,
                        MinValue = 0,
                    };

                    return true;
                });
        }

        public ICommand OpenWebCommand { get; }
    }
}