using FactsApp.Services;
using FactsApp.Views;
using System;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace FactsApp
{
    public partial class App : Application
    {

        public App()
        {
            Device.SetFlags(new string[] { "Shapes_Experimental" });

            InitializeComponent();

            DependencyService.Register<MockDataStore>();
            MainPage = new AppShell();
        }

        protected override void OnStart()
        {
        }

        protected override void OnSleep()
        {
        }

        protected override void OnResume()
        {
        }
    }
}
