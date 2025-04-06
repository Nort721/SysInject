using System;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace ProcessExplorerClone
{
    public class MainForm : Form
    {
        private ListView processListView;
        private ContextMenuStrip contextMenu;
        private ToolStripMenuItem injectMenuItem;
        private ToolStripMenuItem refreshMenuItem;
        private StatusStrip statusBar;
        private ToolStripStatusLabel statusLabel;
        private int fixedWidth = 600;
        private ColumnHeaderSorter listViewSorter;

        public MainForm()
        {
            Text = "SysInject";
            Width = fixedWidth;
            Height = 500;
            Font = SystemFonts.MessageBoxFont;
            StartPosition = FormStartPosition.CenterScreen;
            MinimumSize = new Size(fixedWidth, 300);
            DoubleBuffered = true;

            AllowDrop = true;
            DragEnter += MainForm_DragEnter;
            DragDrop += MainForm_DragDrop;

            KeyPreview = true;
            KeyDown += (s, e) => {
                if (e.KeyCode == Keys.F5) LoadProcessList();
            };

            InitializeMenu();
            InitializeListView();
            InitializeStatusBar();

            LoadProcessList();
        }

        private void InitializeMenu()
        {
            contextMenu = new ContextMenuStrip();

            injectMenuItem = new ToolStripMenuItem("Inject", null, OnInjectClicked);
            refreshMenuItem = new ToolStripMenuItem("Refresh", null, (s, e) => LoadProcessList());

            contextMenu.Items.Add(injectMenuItem);
            contextMenu.Items.Add(new ToolStripSeparator());
            contextMenu.Items.Add(refreshMenuItem);
        }

        private void InitializeListView()
        {
            listViewSorter = new ColumnHeaderSorter();
            processListView = new ListView
            {
                Dock = DockStyle.Fill,
                View = View.Details,
                FullRowSelect = true,
                ContextMenuStrip = contextMenu,
                Sorting = SortOrder.None,
                ListViewItemSorter = listViewSorter
            };

            processListView.Columns.Add("Process Name", 300);
            processListView.Columns.Add("PID", 100);
            processListView.Columns.Add("Memory (MB)", 120);

            processListView.ColumnClick += (s, e) =>
            {
                listViewSorter.Column = e.Column;
                listViewSorter.ToggleSortOrder();
                processListView.Sort();
            };

            Controls.Add(processListView);
        }

        private void InitializeStatusBar()
        {
            statusBar = new StatusStrip();
            statusLabel = new ToolStripStatusLabel();
            statusBar.Items.Add(statusLabel);
            Controls.Add(statusBar);
        }

        private void LoadProcessList()
        {
            processListView.BeginUpdate();
            processListView.Items.Clear();

            var processes = Process.GetProcesses();
            foreach (var proc in processes)
            {
                try
                {
                    double memMb = proc.WorkingSet64 / 1024.0 / 1024.0;

                    var item = new ListViewItem(proc.ProcessName);
                    item.SubItems.Add(proc.Id.ToString());
                    item.SubItems.Add(memMb.ToString("0.0"));
                    item.Tag = proc;
                    processListView.Items.Add(item);
                }
                catch { }
            }

            statusLabel.Text = $"Processes: {processListView.Items.Count}";
            processListView.EndUpdate();
        }

        private void OnInjectClicked(object sender, EventArgs e)
        {
            if (processListView.SelectedItems.Count == 0)
            {
                MessageBox.Show("Please select a process first.", "No Process Selected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            var item = processListView.SelectedItems[0];
            string procName = item.SubItems[0].Text;
            string pid = item.SubItems[1].Text;

            InjectionForm form = new InjectionForm();
            if (form.ShowDialog() == DialogResult.OK)
            {
                string selectedInjection = form.SelectedInjectionType;
                string dllPath = form.SelectedDllPath;

                MessageBox.Show(
                    "Would inject DLL:\n" + dllPath + "\nInto process: " + procName + " (PID " + pid + ")\nUsing method: " + selectedInjection,
                    "Injection Preview", MessageBoxButtons.OK, MessageBoxIcon.Information
                );

                // TODO: Actual injection logic
            }
            form.Dispose();
        }

        private void MainForm_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                var files = (string[])e.Data.GetData(DataFormats.FileDrop);
                if (files.Length > 0 && files[0].EndsWith(".dll", StringComparison.OrdinalIgnoreCase))
                    e.Effect = DragDropEffects.Copy;
            }
        }

        private void MainForm_DragDrop(object sender, DragEventArgs e)
        {
            var files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if (files.Length == 0 || !files[0].EndsWith(".dll", StringComparison.OrdinalIgnoreCase))
                return;

            if (processListView.SelectedItems.Count == 0)
            {
                MessageBox.Show("Please select a process before dropping the DLL.", "No Process Selected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            string dllPath = files[0];
            string procName = processListView.SelectedItems[0].SubItems[0].Text;
            string pid = processListView.SelectedItems[0].SubItems[1].Text;

            MessageBox.Show($"Would inject DLL:\n{dllPath}\nInto process: {procName} (PID {pid})", "Injection Preview", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }

        private class ColumnHeaderSorter : System.Collections.IComparer
        {
            public int Column { get; set; }
            public SortOrder Order { get; private set; } = SortOrder.Ascending;

            public void ToggleSortOrder()
            {
                Order = (Order == SortOrder.Ascending) ? SortOrder.Descending : SortOrder.Ascending;
            }

            public int Compare(object x, object y)
            {
                var item1 = (ListViewItem)x;
                var item2 = (ListViewItem)y;
                string str1 = item1.SubItems[Column].Text;
                string str2 = item2.SubItems[Column].Text;

                if (double.TryParse(str1, out double d1) && double.TryParse(str2, out double d2))
                    return (Order == SortOrder.Ascending ? 1 : -1) * d1.CompareTo(d2);
                else
                    return (Order == SortOrder.Ascending ? 1 : -1) * string.Compare(str1, str2);
            }
        }
    }
}
