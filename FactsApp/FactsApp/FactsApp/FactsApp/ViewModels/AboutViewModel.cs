﻿using System;
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
        private double kneeModelThighLength = 60.0f;
        private double kneeModelBodyLength = 120.0f;
        private double kneeModelFootLength = 25.0f;
        private double kneeModelAngleLineLength = 50.0f;
        private double kneeModelArcDisplacement = 25.0f;

        private int maxRecentAngles = 40;
        private int recentAngleSampleRate = 500; // In per ms - the previous sample will go to 500 ms before
        public int angleValuesHeadIndex { get; private set; } = 0;
        public int angleValuesContentSize { get; private set; } = 0;
        public float[] angleValues { get; private set; } = new float[300000];  // In ms

        private double modelHeight = 0.0f;
        public double ModelHeight
        {
            set
            {
                if (value != modelHeight)
                {
                    modelHeight = value;
                    OnPropertyChanged(nameof(ModelHeight));
                }
            }
            get
            {
                return modelHeight;
            }
        }

        private double modelWidth = 0.0f;
        public double ModelWidth
        {
            set
            {
                if (value != modelWidth)
                {
                    modelWidth = value;
                    OnPropertyChanged(nameof(ModelWidth));
                }
            }
            get
            {
                return modelWidth;
            }
        }

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

        private PointCollection bodyModelPoints = new PointCollection();
        public PointCollection BodyModelPoints
        {
            set
            {
                if (value != bodyModelPoints)
                {
                    bodyModelPoints = value;
                    OnPropertyChanged(nameof(BodyModelPoints));
                }
            }
            get
            {
                return bodyModelPoints;
            }
        }

        private PointCollection angleLinePoints = new PointCollection();
        public PointCollection AngleLinePoints
        {
            set
            {
                if (value != angleLinePoints)
                {
                    angleLinePoints = value;
                    OnPropertyChanged(nameof(AngleLinePoints));
                }
            }
            get
            {
                return angleLinePoints;
            }
        }

        private Brush angleLineColor = Brush.Green;
        public Brush AngleLineColor
        {
            set
            {
                if (value != angleLineColor)
                {
                    angleLineColor = value;
                    OnPropertyChanged(nameof(AngleLineColor));
                }
            }
            get
            {
                return angleLineColor;
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

        private PathGeometry angleArc;
        public PathGeometry AngleArc
        {
            set
            {
                if (value != angleArc)
                {
                    angleArc = value;
                    OnPropertyChanged(nameof(AngleArc));
                }
            }
            get
            {
                return angleArc;
            }
        }

        private EllipseGeometry headGeometry;
        public EllipseGeometry HeadGeometry
        {
            set
            {
                if (value != headGeometry)
                {
                    headGeometry = value;
                    OnPropertyChanged(nameof(HeadGeometry));
                }
            }
            get
            {
                return headGeometry;
            }
        }
        public AboutViewModel()
        {
            Title = "About";
            OpenWebCommand = new Command(async () => await Browser.OpenAsync("https://aka.ms/xamarin-quickstart"));

            Device.StartTimer(TimeSpan.FromSeconds(0.001), () =>
                {
                    angleValues[angleValuesHeadIndex] = angleValuesHeadIndex % 100 + 10;

                    // Only update the UI every so often so we don't overload the work we have to do
                    if (angleValuesHeadIndex % recentAngleSampleRate == 0)
                    {
                        UpdateView();
                    }

                    angleValuesHeadIndex += 1;
                    if (angleValuesHeadIndex >= angleValues.Length)
                    {
                        angleValuesHeadIndex = 0;
                    }

                    if (angleValuesContentSize < angleValues.Length)
                    {
                        angleValuesContentSize += 1;
                    }

                    return true;
                });
        }

        public ICommand OpenWebCommand { get; }

        public void UpdateView()
        {
            Size kneeModelArcRadii = new Size(kneeModelArcDisplacement, kneeModelArcDisplacement);

            double baseModelHeight = kneeModelBodyLength + 2 * kneeModelThighLength + 20.0f;

            PointCollection newBodyPoints = new PointCollection();
            PointCollection newAngleLinePoints = new PointCollection();

            int currWidth = (int)(Application.Current.MainPage.Width);

            FlexionAngleValue = angleValues[angleValuesHeadIndex];

            if (FlexionAngleValue > 150) FlexionAngleValue = 45;

            if (FlexionAngleValue > 90)
            {
                FlexionAngleColor = Color.Red;
                AngleLineColor = Brush.Red;
            }
            else
            {
                FlexionAngleColor = Color.Green;
                AngleLineColor = Brush.Green;
            }

            // Scale this based on the flexion angle to approximate
            double kneeVerticalAngle = FlexionAngleValue * 45 / 180;
            double hipHorizontalAngle = 90 + kneeVerticalAngle - FlexionAngleValue;
            double bodyVerticalAngle = kneeVerticalAngle / 2;


            double ankleX = currWidth / 2;
            double ankleY = baseModelHeight;
            double toeX = ankleX - kneeModelFootLength;
            double toeY = ankleY;
            double kneeY = ankleY - Math.Cos(kneeVerticalAngle * Math.PI / 180) * kneeModelThighLength;
            double kneeX = ankleX - Math.Sin(kneeVerticalAngle * Math.PI / 180) * kneeModelThighLength;
            double hipY = kneeY - Math.Sin(hipHorizontalAngle * Math.PI / 180) * kneeModelThighLength;
            double hipX = kneeX + Math.Cos(hipHorizontalAngle * Math.PI / 180) * kneeModelThighLength;
            double headY = hipY - Math.Cos(bodyVerticalAngle * Math.PI / 180) * kneeModelBodyLength;
            double headX = hipX - Math.Sin(bodyVerticalAngle * Math.PI / 180) * kneeModelBodyLength;

            double kneeLineExtendedX = kneeX - Math.Cos(hipHorizontalAngle * Math.PI / 180) * kneeModelAngleLineLength;
            double kneeLineExtendedY = kneeY + Math.Sin(hipHorizontalAngle * Math.PI / 180) * kneeModelAngleLineLength;

            double angleArcStartX = kneeX - Math.Cos(hipHorizontalAngle * Math.PI / 180) * kneeModelArcDisplacement;
            double angleArcStartY = kneeY + Math.Sin(hipHorizontalAngle * Math.PI / 180) * kneeModelArcDisplacement;
            double angleArcEndX = kneeX + Math.Sin(kneeVerticalAngle * Math.PI / 180) * kneeModelArcDisplacement;
            double angleArcEndY = kneeY + Math.Cos(kneeVerticalAngle * Math.PI / 180) * kneeModelArcDisplacement;

            newBodyPoints.Add(new Point(toeX, toeY));
            newBodyPoints.Add(new Point(ankleX, ankleY));
            newBodyPoints.Add(new Point(kneeX, kneeY));
            newBodyPoints.Add(new Point(hipX, hipY));
            newBodyPoints.Add(new Point(headX, headY));

            newAngleLinePoints.Add(new Point(kneeX, kneeY));
            newAngleLinePoints.Add(new Point(kneeLineExtendedX, kneeLineExtendedY));

            // Have to set this to notify an update in the UI
            BodyModelPoints = newBodyPoints;
            AngleLinePoints = newAngleLinePoints;

            Point angleArcStart = new Point(angleArcStartX, angleArcStartY);
            Point angleArcEnd = new Point(angleArcEndX, angleArcEndY);

            // Create a new path geometry for the arc to dynamically update
            PathGeometry newAngleArc = new PathGeometry();

            ArcSegment newArc = new ArcSegment(angleArcEnd, kneeModelArcRadii, 0, SweepDirection.CounterClockwise, false);
            PathSegmentCollection pathSegments = new PathSegmentCollection();
            pathSegments.Add(newArc);

            PathFigure newPathFig = new PathFigure()
            {
                StartPoint = angleArcStart,
                Segments = pathSegments
            };

            newAngleArc.Figures.Add(newPathFig);

            AngleArc = newAngleArc;

            // Create a new ellipse geometry for the head to dynamically update
            EllipseGeometry newHead = new EllipseGeometry()
            {
                Center = new Point(headX, headY),
                RadiusX = 10,
                RadiusY = 10
            };

            HeadGeometry = newHead;

            List<ChartEntry> recentAngleValues = new List<ChartEntry>();
            int angleIndex = angleValuesHeadIndex;

            // Populate a list with a number of recent angles. The resolution of the full list may not be the same resolution of the receny list, so only a couple of values are sampled.
            while(recentAngleValues.Count < maxRecentAngles)
            {
                recentAngleValues.Insert(0, new ChartEntry(angleValues[angleIndex]));
                angleIndex -= recentAngleSampleRate;

                // If we go past the end, loop back around the index
                if (angleIndex < 0)
                {
                    angleIndex += angleValues.Length;
                }
            }

            RecentAngleChart = new LineChart()
            {
                Entries = recentAngleValues,
                IsAnimated = false,
                AnimationDuration = new TimeSpan(0), // Add this to prevent the chart from redrawing the intro animation every single time
                MaxValue = 180,
                MinValue = 0,
            };

            ModelHeight = baseModelHeight;
            ModelWidth = currWidth;
        }
    }

}