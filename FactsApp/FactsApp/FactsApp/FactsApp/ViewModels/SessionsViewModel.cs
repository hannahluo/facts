using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microcharts;
using System.IO;

using Xamarin.Forms;

namespace FactsApp.ViewModels
{
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
            Device.StartTimer(TimeSpan.FromSeconds(1), () =>
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
                };
                button.Clicked += OnDynamicBtnClicked;
                s.Children.Add(button);
            }
        }

        private void OnDynamicBtnClicked(object sender, EventArgs e)
        {
            var button = sender as Button;
            SessionNameText = button.Text;

            string data = File.ReadAllText(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), SessionNameText + ".csv"));

            var strNumData = data.Split(',');
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
    }
}