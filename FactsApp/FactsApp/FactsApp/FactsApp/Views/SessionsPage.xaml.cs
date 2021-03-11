using System;
using System.Collections.Generic;
using System.ComponentModel;
using FactsApp.ViewModels;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using System.IO;
using Xamarin.Essentials;

namespace FactsApp.Views
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class SessionsPage : ContentPage
    {
        SessionsViewModel m_viewModel;
        public SessionsPage()
        {
            InitializeComponent();
            BindingContext = m_viewModel = new SessionsViewModel(AllFiles);
        }

        protected override void OnAppearing()
        {
            base.OnAppearing();
            m_viewModel.UpdateView(AllFiles);
        }
    }
}