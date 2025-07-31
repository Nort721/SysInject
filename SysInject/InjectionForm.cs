using System;
using System.Drawing;
using System.Windows.Forms;

public class InjectionForm : Form
{
    private ComboBox injectionTypeComboBox;
    private TextBox dllPathTextBox;
    private Button browseButton;
    private Button injectButton;
    private Button cancelButton;
    private Label injectionTypeLabel;
    private Label dllPathLabel;

    public string SelectedInjectionType
    {
        get { return injectionTypeComboBox.SelectedItem?.ToString() ?? ""; }
    }

    public string SelectedDllPath
    {
        get { return dllPathTextBox.Text; }
    }

    public InjectionForm()
    {
        Text = "Inject DLL";
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        StartPosition = FormStartPosition.CenterParent;
        ClientSize = new Size(420, 140);
        Font = SystemFonts.MessageBoxFont;
        DoubleBuffered = true;

        injectionTypeLabel = new Label();
        injectionTypeLabel.Text = "Injection Method:";
        injectionTypeLabel.Location = new Point(12, 15);
        injectionTypeLabel.AutoSize = true;

        injectionTypeComboBox = new ComboBox();
        injectionTypeComboBox.DropDownStyle = ComboBoxStyle.DropDownList;
        injectionTypeComboBox.Items.AddRange(new object[] {
            "LoadLibrary",
            "Manual Map",
            "Thread Hijack"
        });
        injectionTypeComboBox.SelectedIndex = 0;
        injectionTypeComboBox.Location = new Point(130, 12);
        injectionTypeComboBox.Size = new Size(270, 24);
        // Add the event handler
        injectionTypeComboBox.SelectedIndexChanged += InjectionTypeComboBox_SelectedIndexChanged;

        dllPathLabel = new Label();
        dllPathLabel.Text = "DLL to Inject:";
        dllPathLabel.Location = new Point(12, 55);
        dllPathLabel.AutoSize = true;

        dllPathTextBox = new TextBox();
        dllPathTextBox.Location = new Point(130, 52);
        dllPathTextBox.Size = new Size(190, 23);
        dllPathTextBox.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;

        browseButton = new Button();
        browseButton.Text = "Browse...";
        browseButton.Location = new Point(330, 50);
        browseButton.Size = new Size(70, 27);
        browseButton.Click += new EventHandler(BrowseButton_Click);

        injectButton = new Button();
        injectButton.Text = "Inject";
        injectButton.Location = new Point(230, 100);
        injectButton.Size = new Size(80, 30);
        injectButton.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
        injectButton.Click += new EventHandler(InjectButton_Click);

        cancelButton = new Button();
        cancelButton.Text = "Cancel";
        cancelButton.Location = new Point(320, 100);
        cancelButton.Size = new Size(80, 30);
        cancelButton.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
        cancelButton.DialogResult = DialogResult.Cancel;

        Controls.Add(injectionTypeLabel);
        Controls.Add(injectionTypeComboBox);
        Controls.Add(dllPathLabel);
        Controls.Add(dllPathTextBox);
        Controls.Add(browseButton);
        Controls.Add(injectButton);
        Controls.Add(cancelButton);

        AcceptButton = injectButton;
        CancelButton = cancelButton;
    }

    private void InjectionTypeComboBox_SelectedIndexChanged(object sender, EventArgs e)
    {
        if (injectionTypeComboBox.SelectedItem.ToString() == "Manual Map")
        {
            MessageBox.Show("Make sure your DLL is compatible.", "Compatibility Note", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }

    private void BrowseButton_Click(object sender, EventArgs e)
    {
        OpenFileDialog dlg = new OpenFileDialog();
        dlg.Filter = "DLL files (*.dll)|*.dll|All files (*.*)|*.*";
        dlg.Title = "Select DLL to Inject";

        if (dlg.ShowDialog() == DialogResult.OK)
        {
            dllPathTextBox.Text = dlg.FileName;
        }
    }

    private void InjectButton_Click(object sender, EventArgs e)
    {
        if (string.IsNullOrEmpty(dllPathTextBox.Text))
        {
            MessageBox.Show("Please select a DLL to inject.", "Missing DLL", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        DialogResult = DialogResult.OK;

        Close();
    }
}
