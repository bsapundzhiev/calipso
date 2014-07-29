using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using System.Windows.Data;
using System.Collections.ObjectModel;
using Microsoft.Phone.Storage;
using System.ComponentModel;

namespace CalipsoWP.Controls
{
    public class FileExplorerItem
    {
        public string Name { get; set; }
        public string Path { get; set; }
        public bool IsFolder { get; set; }

        public override string ToString()
        {
            return Name;
        }
    }

    public class ExplorerTypeToIconConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            bool isFolder = (bool)value;
            return string.Format("/Assets/Icons/{0}", isFolder ? "folder.png" : "file.png");
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public partial class FilePicker : UserControl
    {
        public ObservableCollection<FileExplorerItem> CurrentItems { get; set; }
        //public string CurrentPath { get; set; }
        public static readonly DependencyProperty TextProperty =
        DependencyProperty.Register(
           "CurrentPath",
           typeof(string),
           typeof(FilePicker),
           new PropertyMetadata(null));

        public string CurrentPath
        {
            get { return (string)GetValue(TextProperty); }
            set
            {
                SetValue(TextProperty, value);
            }
        }

        PhoneApplicationPage _currentPage;
        PhoneApplicationFrame _currentFrame;
        ExternalStorageDevice _currentStorageDevice;
        Stack<ExternalStorageFolder> _folderTree { get; set; }
        bool _mustRestoreApplicationBar = false;
        bool _mustRestoreSystemTray = false;

        public event OnDismissEventHandler OnDismiss;
        public delegate void OnDismissEventHandler(ExternalStorageFile file);

        public FilePicker()
        { 
            InitializeComponent();
            Initialize();
        }

        async void Initialize()
        {
            _folderTree = new Stack<ExternalStorageFolder>();
            CurrentItems = new ObservableCollection<FileExplorerItem>();

            var storageAssets = await ExternalStorage.GetExternalStorageDevicesAsync();
            _currentStorageDevice = storageAssets.FirstOrDefault();

            LayoutRoot.Width = Application.Current.Host.Content.ActualWidth;
            LayoutRoot.Height = Application.Current.Host.Content.ActualHeight;

            if (_currentStorageDevice != null)
                GetTreeForFolder(_currentStorageDevice.RootFolder);
        }

        async void GetTreeForFolder(ExternalStorageFolder folder)
        {
            CurrentItems.Clear();

            var folderList = await folder.GetFoldersAsync();

            foreach (ExternalStorageFolder _folder in folderList)
            {
                CurrentItems.Add(new FileExplorerItem() { IsFolder = true, Name = _folder.Name, Path = _folder.Path });
            }

            foreach (ExternalStorageFile _file in await folder.GetFilesAsync())
            {
                CurrentItems.Add(new FileExplorerItem() { IsFolder = false, Name = _file.Name, Path = _file.Path });
            }

            if (!_folderTree.Contains(folder))
                _folderTree.Push(folder);

            CurrentPath = _folderTree.First().Path;
            
        }

        void TreeUp(object sender, RoutedEventArgs e)
        {
            if (_folderTree.Count > 1)
            {
                _folderTree.Pop();
                GetTreeForFolder(_folderTree.First());
            }
        }

        public void Show()
        {
            _currentFrame = Application.Current.RootVisual as PhoneApplicationFrame;
            _currentPage = _currentFrame.Content as PhoneApplicationPage;

            if (SystemTray.IsVisible)
            {
                _mustRestoreSystemTray = true;
                SystemTray.IsVisible = false;
            }


            if (_currentPage.ApplicationBar != null)
            {
                if (_currentPage.ApplicationBar.IsVisible)
                    _mustRestoreApplicationBar = true;

                _currentPage.ApplicationBar.IsVisible = false;
            }

            if (_currentPage != null)
            {
                _currentPage.BackKeyPress += OnBackKeyPress;
            }

            RootPopup.IsOpen = true;
        }

        void OnBackKeyPress(object sender, CancelEventArgs e)
        {
            e.Cancel = true;
            Dismiss(null);
        }

        private void Dismiss(ExternalStorageFile file)
        {
            if (_currentPage != null)
            {
                _currentPage.BackKeyPress -= OnBackKeyPress;
            }

            RootPopup.IsOpen = false;

            if (_mustRestoreApplicationBar)
                _currentPage.ApplicationBar.IsVisible = true;

            if (_mustRestoreSystemTray)
                SystemTray.IsVisible = true;

            if (OnDismiss != null)
                OnDismiss(file);
        }

        async void SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (lstCore.SelectedItem != null)
            {
                FileExplorerItem item = (FileExplorerItem)lstCore.SelectedItem;
                if (item.IsFolder)
                {
                    GetTreeForFolder(await _folderTree.First().GetFolderAsync(item.Name));
                }
                else
                {
                    ExternalStorageFile file = await _currentStorageDevice.GetFileAsync(item.Path);
                    Dismiss(file);
                }
            }
        }

    }
}
