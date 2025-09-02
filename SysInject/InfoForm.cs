using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

namespace SysInject
{
    public partial class InfoForm : Form
    {
        private uint SelectedPid;

        public InfoForm(uint SelectedPid)
        {
            InitializeComponent();
            this.SelectedPid = SelectedPid;
        }

        private void InfoForm_Load(object sender, EventArgs e)
        {
            var proc = Process.GetProcessById((int)SelectedPid);

            // Basic Identity (TODO finish implementing data)
            listView1.Items.Add(new ListViewItem(new[] { "Process Name", proc.ProcessName }));
            listView1.Items.Add(new ListViewItem(new[] { "PID", proc.Id.ToString() }));
            listView1.Items.Add(new ListViewItem(new[] { "Parent PID", "" }));
            listView1.Items.Add(new ListViewItem(new[] { "Executable Path", "" }));
            listView1.Items.Add(new ListViewItem(new[] { "File Type", "" }));
            listView1.Items.Add(new ListViewItem(new[] { "Command Line", "" }));
            listView1.Items.Add(new ListViewItem(new[] { "User / Owner", "" }));

            // Memory Usage
            listView2.Items.Add(new ListViewItem(new[] { "Working Set", ProcessInfoHelper.GetWorkingSet(proc.Id) }));
            listView2.Items.Add(new ListViewItem(new[] { "Private Bytes", ProcessInfoHelper.GetPrivateBytes(proc.Id) }));
            listView2.Items.Add(new ListViewItem(new[] { "Virtual Memory", ProcessInfoHelper.GetVirtualMemory(proc.Id) }));
            listView2.Items.Add(new ListViewItem(new[] { "Paged Memory", ProcessInfoHelper.GetPagedMemory(proc.Id) }));
            listView2.Items.Add(new ListViewItem(new[] { "Nonpaged Memory", ProcessInfoHelper.GetNonpagedMemory(proc.Id) }));

            // Security & Integrity (TODO finish implementing data)
            listView3.Items.Add(new ListViewItem(new[] { "Integrity Level", "" }));
            listView3.Items.Add(new ListViewItem(new[] { "Digital Signature", "" }));
            listView3.Items.Add(new ListViewItem(new[] { "ASLR Status", "" }));
            listView3.Items.Add(new ListViewItem(new[] { "DEP Status", "" }));
            listView3.Items.Add(new ListViewItem(new[] { "Sandboxed", "" }));

        }
    }
}
