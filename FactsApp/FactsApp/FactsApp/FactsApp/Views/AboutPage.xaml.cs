using System;
using System.Collections.Generic;
using System.ComponentModel;
using FactsApp.ViewModels;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

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
    }
}