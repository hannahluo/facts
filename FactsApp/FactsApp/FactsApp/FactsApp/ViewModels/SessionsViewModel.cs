using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microcharts;
using System.IO;

using Xamarin.Forms;

namespace FactsApp.ViewModels
{
    public class DataButton : Button
    {
        public string data = "";
    }

    public class SessionsViewModel : BaseViewModel
    {
        private string sessionNameText = "None";
        public string SessionNameText
        {
            set
            {
                if (value != sessionNameText)
                {
                    sessionNameText = value;
                    OnPropertyChanged(nameof(SessionNameText));
                }
            }
            get
            {
                return sessionNameText;
            }
        }

        private Chart angleChart;
        public Chart AngleChart
        {
            set
            {
                if (value != angleChart)
                {
                    angleChart = value;
                    OnPropertyChanged(nameof(AngleChart));
                }
            }
            get
            {
                return angleChart;
            }
        }

        public SessionsViewModel(StackLayout s)
        {
            Device.StartTimer(TimeSpan.FromSeconds(0.5), () =>
            {
                UpdateView(s);
                return true;
            });
        }

        public void UpdateView(StackLayout s)
        {
            FileInfo[] files = new DirectoryInfo(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData).ToString()).GetFiles("*.csv");

            s.Children.Clear(); 
            foreach (var file in files)
            {
                var button = new Button()
                {
                    Text = Path.GetFileNameWithoutExtension(file.Name),
                    HorizontalOptions = LayoutOptions.Center,
                };
                button.Clicked += OpenSession;

                var deleteButton = new DataButton()
                {
                    Text = "Delete",
                    HorizontalOptions = LayoutOptions.Center,
                };
                deleteButton.Clicked += DeleteSession;

                deleteButton.data = Path.GetFileNameWithoutExtension(file.Name);

                var horizontalLayout = new StackLayout
                {
                    Margin = new Thickness(5),
                    Orientation = StackOrientation.Horizontal,
                    HorizontalOptions = LayoutOptions.Center,
                    Children =
                    {
                        button,
                        deleteButton,
                    }
                };

                s.Children.Add(horizontalLayout);
            }
        }

        private void OpenSession(object sender, EventArgs e)
        {
            var button = sender as Button;
            SessionNameText = button.Text;

            string data = File.ReadAllText(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), SessionNameText + ".csv"));

            string[] strNumData = data.Split(',');

            // If the user just clicks without any data saved or some other error occurs, make sure we have some data to interpret as a float or the parse call will fail
            if (strNumData.Length  < 2)
            {
                strNumData = new string[] {"0"};
            }
            List<ChartEntry> newAngleValues = new List<ChartEntry>();

            for (int i=0; i < strNumData.Length; ++i)
            {
                newAngleValues.Add(new ChartEntry(float.Parse(strNumData[i])));
            }
            
            AngleChart = new LineChart()
            {
                Entries = newAngleValues,
                IsAnimated = false,
                MaxValue = 180,
                MinValue = 0,
            };
        }

        private void DeleteSession(object sender, EventArgs e)
        {
            var button = sender as DataButton;
            SessionNameText = "";

            File.Delete(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), button.data + ".csv"));

            AngleChart = new LineChart()
            {
                IsAnimated = false,
                MaxValue = 180,
                MinValue = 0,
            };
        }
    }
}