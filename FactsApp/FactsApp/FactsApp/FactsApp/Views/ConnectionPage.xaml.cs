using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FactsApp.ViewModels;

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
            BindingContext = m_viewModel = new ConnectionViewModel();
        }

        public void OnButtonClicked(object sender, EventArgs e)
        {
            m_viewModel.OnButtonClicked(sender, e);
        }
    }
}