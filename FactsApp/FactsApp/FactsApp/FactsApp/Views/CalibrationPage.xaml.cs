using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FactsApp.ViewModels;
using Acr.UserDialogs;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace FactsApp.Views
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class CalibrationPage : ContentPage
    {
        CalibrationViewModel m_viewModel;
        public CalibrationPage()
        {
            InitializeComponent();
            BindingContext = m_viewModel = new CalibrationViewModel(UserDialogs.Instance);
        }

        public void OnCalibrateButtonClicked(object sender, EventArgs e)
        {
            m_viewModel.OnCalibrateButtonClicked(sender, e);
        }

        protected override void OnAppearing()
        {
            m_viewModel.OnAppearing();
        }
    }
}