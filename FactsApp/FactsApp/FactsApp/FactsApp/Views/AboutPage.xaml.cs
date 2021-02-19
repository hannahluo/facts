using System;
using System.Collections.Generic;
using System.ComponentModel;
using FactsApp.ViewModels;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using System.IO;

namespace FactsApp.Views
{
    public partial class AboutPage : ContentPage
    {
        AboutViewModel m_viewModel;
        public AboutPage()
        {
            InitializeComponent();
            BindingContext = m_viewModel = new AboutViewModel();
        }

        protected override void OnAppearing()
        {
            base.OnAppearing();
            m_viewModel.UpdateView();
        }

        async void SaveDataToFile(object sender, EventArgs args)
        {
            // Use current date and time to ensure no overwriting existing files
            string fileName = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "FactsData" + System.DateTime.Now.ToString("yyyy-dd-M--HH-mm-ss") + ".csv");

            using (StreamWriter writer = File.AppendText(fileName))
            {
                for (int i = 0; i < m_viewModel.angleValues.Length; ++i)
                {
                    writer.Write((i / 1000.0f) + ",");
                }

                writer.Write(Environment.NewLine);

                int dataIndex = m_viewModel.angleValuesHeadIndex;

                for (int i = 0; i < m_viewModel.angleValuesContentSize; ++i)
                {
                    writer.Write(m_viewModel.angleValues[dataIndex] + ",");
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