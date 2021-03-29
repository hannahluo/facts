using System;
using System.Collections.Generic;
using System.ComponentModel;
using FactsApp.ViewModels;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using System.IO;
using Xamarin.Essentials;
using Acr.UserDialogs;

namespace FactsApp.Views
{
    public partial class AboutPage : ContentPage
    {
        AboutViewModel m_viewModel;
        public AboutPage()
        {
            InitializeComponent();
            BindingContext = m_viewModel = new AboutViewModel(UserDialogs.Instance);
        }

        protected override void OnAppearing()
        {
            base.OnAppearing();
            m_viewModel.UpdateView();
            m_viewModel.StartAngleReading();
        }

        protected override void OnDisappearing()
        {
            base.OnDisappearing();
            m_viewModel.StopAngleReading();
        }

        async void SaveDataToFile(object sender, EventArgs args)
        {
            // Use current date and time to ensure no overwriting existing files
            string fileName = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), System.DateTime.Now.ToString("yyyy-dd-M--HH-mm-ss") + ".csv");

            using (StreamWriter writer = File.AppendText(fileName))
            {
                int dataIndex = m_viewModel.angleValuesHeadIndex - m_viewModel.angleValuesContentSize;
                if (dataIndex < 0)
                    dataIndex += m_viewModel.angleValues.Length;

                for (int i = 0; i < m_viewModel.angleValuesContentSize; ++i)
                {
                    // Avoid the trailing comma on the last entry so we can read this easier
                    if (i == m_viewModel.angleValuesContentSize - 1)
                    {
                        writer.Write(m_viewModel.angleValues[dataIndex]);
                    }
                    else
                    {
                        writer.Write(m_viewModel.angleValues[dataIndex] + ",");
                    }
                    dataIndex += 1;
                    if (dataIndex >= m_viewModel.angleValues.Length)
                    {
                        dataIndex = 0;
                    }
                }
            }
        }
    }
}