using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using CalipsoWP.Resources;
using CalipsoPhoneRuntimeComponent;
using Windows.Networking.Connectivity;
using System.Text;
using Windows.Storage;
using System.Threading.Tasks;
using Microsoft.Phone.Storage;
using System.IO;
using System.Collections.ObjectModel;
using CalipsoWP.Controls;

namespace CalipsoWP
{
    public partial class MainPage : PhoneApplicationPage
    {
        //TODO:
        ObservableCollection<String> _list;

        WindowsPhoneRuntimeComponent component;
        FilePicker _filePicker;
        
        // Constructor
        public MainPage()
        {
            InitializeComponent();

            // Sample code to localize the ApplicationBar
            //BuildLocalizedApplicationBar();
            ///init calipso compoment
            component = new WindowsPhoneRuntimeComponent();
            component.Start();
            _filePicker = new FilePicker();
            _filePicker.OnDismiss += filePicker_OnDismiss;

            _list = new ObservableCollection<String>();
        }

        async void filePicker_OnDismiss(ExternalStorageFile file)
        {
            if (file != null)
            {
                StatusBox.Text = "Wait...";
               // string incomingRouteFilename = Windows.Phone.Storage.SharedAccess.SharedStorageAccessManager.GetSharedFileName(file.Name);
               // StatusBox.Text = incomingRouteFilename;
                Stream photoToSave = await file.OpenForReadAsync();
                StorageFolder localFolder = ApplicationData.Current.LocalFolder;
                StorageFile photoFile = await localFolder.CreateFileAsync(file.Name, CreationCollisionOption.ReplaceExisting);
                using (var photoOutputStream = await photoFile.OpenStreamForWriteAsync())
                {
                    await photoToSave.CopyToAsync(photoOutputStream);
                }
                StatusBox.Text = file.Path + file.Name;
            }
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);


            var hostnames = NetworkInformation.GetHostNames();
            _list.Clear();
            foreach (var hn in hostnames)
            {
                if (hn.IPInformation != null)
                {

                    string ipAddress = hn.DisplayName;
                    string name = hn.CanonicalName;
                    string formatedUrl = string.Format("http://{0}/", name);
                    _list.Add(formatedUrl);
                }   
            }

            HostList.ItemsSource = _list;
           
            //storage
            //StorageFolder storageFolder = KnownFolders.DocumentsLibrary; 
            //PathBox.Text = KnownFolders.SavedPictures.Path;
            //PathBox.Text = Windows.Storage.KnownFolders.RemovableDevices.Path;
            StorageFolder local = Windows.Storage.ApplicationData.Current.LocalFolder;
            PathBox.Text = local.Path;
        }

       
        private void ApplicationBarMenuItem_Click(object sender, EventArgs e)
        {
            _filePicker.Show();
        }


        // Sample code for building a localized ApplicationBar
        //private void BuildLocalizedApplicationBar()
        //{
        //    // Set the page's ApplicationBar to a new instance of ApplicationBar.
        //    ApplicationBar = new ApplicationBar();

        //    // Create a new button and set the text value to the localized string from AppResources.
        //    ApplicationBarIconButton appBarButton = new ApplicationBarIconButton(new Uri("/Assets/AppBar/appbar.add.rest.png", UriKind.Relative));
        //    appBarButton.Text = AppResources.AppBarButtonText;
        //    ApplicationBar.Buttons.Add(appBarButton);

        //    // Create a new menu item with the localized string from AppResources.
        //    ApplicationBarMenuItem appBarMenuItem = new ApplicationBarMenuItem(AppResources.AppBarMenuItemText);
        //    ApplicationBar.MenuItems.Add(appBarMenuItem);
        //}
    }
}