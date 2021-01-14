using FactsApp.ViewModels;
using System.ComponentModel;
using Xamarin.Forms;

namespace FactsApp.Views
{
    public partial class ItemDetailPage : ContentPage
    {
        public ItemDetailPage()
        {
            InitializeComponent();
            BindingContext = new ItemDetailViewModel();
        }
    }
}