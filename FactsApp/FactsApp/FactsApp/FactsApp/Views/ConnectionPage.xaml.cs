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
    public partial class ConnectionPage : ContentPage
    {
        ConnectionViewModel m_viewModel;
        public ConnectionPage()
        {
            InitializeComponent();
            BindingContext = m_viewModel = new ConnectionViewModel(UserDialogs.Instance);
        }

        public void OnScanButtonClicked(object sender, EventArgs e)
        {
            m_viewModel.OnScanButtonClicked(sender, e);
        }

        public void OnListSelection(object sender, SelectedItemChangedEventArgs e)
        {
            m_viewModel.OnConnectionSelect(e);
        }

        public void OnDisconnectButtonClicked(object sender, EventArgs e)
        {
            m_viewModel.OnDisconnectButtonClicked(sender, e);
        }
    }
}