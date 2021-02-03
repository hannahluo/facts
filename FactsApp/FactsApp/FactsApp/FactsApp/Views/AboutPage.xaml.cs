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
        public AboutPage()
        {
            InitializeComponent();
            BindingContext = new AboutViewModel();
        }
    }
}