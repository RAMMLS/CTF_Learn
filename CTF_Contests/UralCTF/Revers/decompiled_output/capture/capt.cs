using System;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Windows.Forms;
using capture.Properties;

namespace capture;

public class capt : Form
{
	private class L
	{
		private ulong A = 1664525uL;

		private ulong Cc = 1013904223uL;

		private ulong M = 4294967296uL;

		private ulong s;

		public L(long seed)
		{
			s = (ulong)seed;
		}

		public long Next()
		{
			s = (A * s + Cc) % M;
			return (long)s;
		}
	}

	private readonly string[] B = new string[1] { "Wk2QAAMAAAAEAAAA//8AALgAAAAAAAAAQ" + new string('A', 47) + "sAAAAA4fug4AtAnNIbgBTM0hVGhpcyBwcm9ncmFtIGNhbm5vdCBiZSBydW4gaW4gRE9TIG1vZGUuDQ0KJAAAAAAAAAC9D8va+W6liflupYn5bqWJd3G2if1upYkFTreJ+G6liVJpY2j5bqWJ" + new string('A', 10) + "BQRQAATAEEAP5/z2g" + new string('A', 10) + "OAADwELAQUMAAIAAAAGAAAAAAAAAEAAAAAQAAAAIAAAAABAAAAQAAAAAgAAB" + new string('A', 10) + "E" + new string('A', 10) + "BQAAAABAAAAAAAAAIAAAAAABAAABAAAAAAEAAAEAAAAAAAAB" + new string('A', 10) + "AAAAAAggAAAo" + new string('A', 68) + new string('A', 44) + "IAAAC" + new string('A', 31) + "AAAAAC50ZXh0AAAABgAAAAAQAAAAAgAAAAQ" + new string('A', 18) + "CAAAGAucmRhdGEAAFIAAAAAIAAAAAIAAAAG" + new string('A', 18) + "BAAABAREFUQQAAAADvAAAAADAAAAACAAAAC" + new string('A', 19) + "QAAAwENPREUAAAAAFAAAAABAAAAAAgAAAAo" + new string('A', 18) + "EAAAM" + new string('A', 22) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + "AAAAAP8lACB" + new string('A', 69) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 48) + "OCAAAAAAAAAwI" + new string('A', 13) + "BGIAAAAC" + new string('A', 30) + "OCAAAAAAAACxAU1lc3NhZ2VCb3hBAHVzZXIzMi5kbGwAAAAA" + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 10) + "BGaW5hbCBTdGFnZQBXZWxjb21lIHRvIHRoZSBGaW5hbCBTdGFnZSEAFjMxGDYmIzYRci8pKxEAVT0VcDMjK0QcOiAcHCdgBhhHGENhUHR1cmVNZQBDYVB0VXJlTWUAQ2FQdHVyRU1lAKMbXH0+Kv8QZwARIjNEVWZ3iJkAQ0FQdHVyZU1lAExBVVRdKzx/agChssPU5fYXKDkAQ2FwdHVyZU1lAP/spyJdEytNb3wAVYvsg+wQagxoEAAAkJCQzMyLRQyJRQQAuAQAAADNIZCQAGNhcHR1cmVtZQBDQVBUVVJFTUUAQ0FQdHVyZW1l" + new string('A', 72) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 53) + "GoAaAAwQABoDDBAAGoA6O3P///D" + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 80) + new string('A', 16) };

	private bool K;

	private string S = "";

	private L G;

	private const string PPHR_B64 = "Y2FwdHVyZV9tYXN0ZXJfa2V5XzIwMjUh";

	private const string SL_B64 = "IUNlh6nL7Q8=";

	private const string OBFSCR = "H0o7a48=";

	private const string M4SK = "TyF6EcM=";

	private IContainer components;

	private Button button1;

	private Label label1;

	private TextBox textBox1;

	private TextBox textBox2;

	private Label label2;

	private Button button2;

	public capt()
	{
		//IL_03b0: Unknown result type (might be due to invalid IL or missing references)
		//IL_03ba: Expected O, but got Unknown
		//IL_03c7: Unknown result type (might be due to invalid IL or missing references)
		//IL_03d1: Expected O, but got Unknown
		//IL_03f5: Unknown result type (might be due to invalid IL or missing references)
		//IL_03ff: Expected O, but got Unknown
		//IL_040c: Unknown result type (might be due to invalid IL or missing references)
		//IL_0416: Expected O, but got Unknown
		InitializeComponent();
		G = new L(12345L);
		((Control)textBox1).KeyDown += new KeyEventHandler(textBox1_KeyDown);
		((Control)textBox1).KeyPress += new KeyPressEventHandler(textBox1_KeyPress);
		((Control)textBox1).TextChanged += textBox1_TextChanged;
		((Control)textBox2).KeyDown += new KeyEventHandler(textBox2_KeyDown);
		((Control)textBox2).KeyPress += new KeyPressEventHandler(textBox2_KeyPress);
		((Control)textBox2).TextChanged += textBox2_TextChanged;
	}

	private void button1_Click(object s, EventArgs e)
	{
		string text = ((Control)textBox1).Text?.Trim();
		if (Z(text))
		{
			((Control)label1).Text = "Пароль принят";
			K = true;
			string text2 = "123e4567-e89b-12d3-a456-" + 4 + 2661 + "4174000";
			long seed = GSFPR(text, text2);
			S = O(text2, seed);
			try
			{
				((Control)textBox2).Text = Encr4Displ(S);
			}
			catch
			{
				((Control)textBox2).Text = Convert.ToBase64String(Encoding.UTF8.GetBytes(S));
			}
		}
		else
		{
			((Control)label1).Text = "Неверный пароль!";
			K = false;
			((Control)textBox2).Text = string.Empty;
		}
		((Control)textBox1).Text = string.Empty;
	}

	private void button2_Click(object s, EventArgs e)
	{
		string uuid = ((Control)textBox2).Text?.Trim();
		((Control)textBox2).Text = string.Empty;
		if (!K || ENCRUUID(uuid) != ENCRUUID(S))
		{
			((Control)label2).Text = "Некорректный UUID!";
			return;
		}
		W();
		((Control)label2).Text = "Куда же он подевался...";
	}

	private bool Z(string k)
	{
		string text = DS();
		if (k == null || k.Length != text.Length)
		{
			return false;
		}
		return X(k) == text;
	}

	private string X(string t)
	{
		byte[] array = new byte[13]
		{
			5, 6, 7, 8, 9, 8, 7, 6, 7, 8,
			9, 8, 9
		};
		char[] array2 = t.ToCharArray();
		char[] array3 = new char[array2.Length];
		int num = 0;
		for (int i = 0; i < array2.Length; i++)
		{
			array3[i] = (char)(array2[i] ^ array[num]);
			num = (num + 1) % array.Length;
		}
		return new string(array3);
	}

	private string ENCRUUID(string uuid)
	{
		if (uuid == null)
		{
			return null;
		}
		char[] array = uuid.ToCharArray();
		for (int i = 0; i < array.Length; i++)
		{
			array[i] = (char)(((uint)array[i] << 3) ^ (uint)(i + 66));
		}
		return new string(array);
	}

	private string Encr4Displ(string plainText)
	{
		if (plainText == null)
		{
			return null;
		}
		string password = Encoding.UTF8.GetString(Convert.FromBase64String("Y2FwdHVyZV9tYXN0ZXJfa2V5XzIwMjUh"));
		byte[] salt = Convert.FromBase64String("IUNlh6nL7Q8=");
		using Rfc2898DeriveBytes rfc2898DeriveBytes = new Rfc2898DeriveBytes(password, salt, 20000);
		byte[] bytes = rfc2898DeriveBytes.GetBytes(32);
		using Aes aes = Aes.Create();
		aes.KeySize = 256;
		aes.Key = bytes;
		aes.Mode = CipherMode.CBC;
		aes.Padding = PaddingMode.PKCS7;
		aes.GenerateIV();
		byte[] iV = aes.IV;
		using MemoryStream memoryStream = new MemoryStream();
		using CryptoStream cryptoStream = new CryptoStream(memoryStream, aes.CreateEncryptor(), CryptoStreamMode.Write);
		byte[] bytes2 = Encoding.UTF8.GetBytes(plainText);
		cryptoStream.Write(bytes2, 0, bytes2.Length);
		cryptoStream.FlushFinalBlock();
		byte[] array = memoryStream.ToArray();
		byte[] array2 = new byte[iV.Length + array.Length];
		Buffer.BlockCopy(iV, 0, array2, 0, iV.Length);
		Buffer.BlockCopy(array, 0, array2, iV.Length, array.Length);
		return Convert.ToBase64String(array2);
	}

	private string DS()
	{
		try
		{
			byte[] array = Convert.FromBase64String("H0o7a48=");
			byte[] array2 = Convert.FromBase64String("TyF6EcM=");
			int num = Math.Min(array.Length, array2.Length);
			char[] array3 = new char[num];
			for (int i = 0; i < num; i++)
			{
				byte b = (byte)(array[i] ^ array2[i]);
				array3[i] = (char)b;
			}
			return new string(array3);
		}
		catch
		{
			return string.Empty;
		}
	}

	private long GSFPR(string password, string raw)
	{
		using SHA256 sHA = SHA256.Create();
		byte[] bytes = Encoding.UTF8.GetBytes((password ?? string.Empty) + "|" + raw);
		return (long)BitConverter.ToUInt64(sHA.ComputeHash(bytes), 0);
	}

	private string O(string u, long seed)
	{
		L l = new L(seed);
		char[] array = u.ToCharArray();
		char[] array2 = new char[array.Length];
		for (int i = 0; i < array.Length; i++)
		{
			if (array[i] == '-')
			{
				array2[i] = '-';
				continue;
			}
			int num = (int)(l.Next() % 26);
			array2[i] = (char)(97 + (array[i] + num) % 26);
		}
		return new string(array2);
	}

	private string O(string u)
	{
		return O(u, 12345L);
	}

	private string C(string b64)
	{
		return Encoding.UTF8.GetString(Convert.FromBase64String(b64));
	}

	private void W()
	{
		string text = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "cache", "qt-installer-framework");
		if (!Directory.Exists(text))
		{
			Directory.CreateDirectory(text);
		}
		string path = Path.Combine(text, "qt-installer-framework.cache");
		byte[] bytes = Convert.FromBase64String(Json());
		File.WriteAllBytes(path, bytes);
	}

	private string Json()
	{
		return string.Concat(B).ToString() ?? string.Format(string.Empty, ((ScrollProperties)((ScrollableControl)this).VerticalScroll).Value);
	}

	private string D(string s)
	{
		return s ?? string.Format(string.Empty.ToLowerInvariant(), ((ScrollProperties)((ScrollableControl)this).HorizontalScroll).Minimum);
	}

	private void textBox1_KeyDown(object sender, KeyEventArgs e)
	{
		//IL_0001: Unknown result type (might be due to invalid IL or missing references)
		//IL_0008: Invalid comparison between Unknown and I4
		if ((int)e.KeyCode == 13)
		{
			e.SuppressKeyPress = true;
			e.Handled = true;
		}
	}

	private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
	{
		if (e.KeyChar == '\r' || e.KeyChar == '\n')
		{
			e.Handled = true;
		}
	}

	private void textBox1_TextChanged(object sender, EventArgs e)
	{
		int selectionStart = ((TextBoxBase)textBox1).SelectionStart;
		string text = ((Control)textBox1).Text.Replace("\r", "").Replace("\n", "");
		if (text != ((Control)textBox1).Text)
		{
			((Control)textBox1).Text = text;
			((TextBoxBase)textBox1).SelectionStart = Math.Min(selectionStart, text.Length);
		}
	}

	private void textBox2_KeyDown(object sender, KeyEventArgs e)
	{
		//IL_0001: Unknown result type (might be due to invalid IL or missing references)
		//IL_0008: Invalid comparison between Unknown and I4
		if ((int)e.KeyCode == 13)
		{
			e.SuppressKeyPress = true;
			e.Handled = true;
		}
	}

	private void textBox2_KeyPress(object sender, KeyPressEventArgs e)
	{
		if (e.KeyChar == '\r' || e.KeyChar == '\n')
		{
			e.Handled = true;
		}
	}

	private void textBox2_TextChanged(object sender, EventArgs e)
	{
		int selectionStart = ((TextBoxBase)textBox2).SelectionStart;
		string text = ((Control)textBox2).Text.Replace("\r", "").Replace("\n", "");
		if (text != ((Control)textBox2).Text)
		{
			((Control)textBox2).Text = text;
			((TextBoxBase)textBox2).SelectionStart = Math.Min(selectionStart, text.Length);
		}
	}

	private void button1_MouseHover(object sender, EventArgs e)
	{
		((Control)button1).BackColor = Color.LightSlateGray;
	}

	private void button2_MouseHover(object sender, EventArgs e)
	{
		((Control)button2).BackColor = Color.LightSlateGray;
	}

	private void button1_BackColorChanged(object sender, EventArgs e)
	{
		((Control)button1).BackColor = Color.White;
	}

	private void button2_BackColorChanged(object sender, EventArgs e)
	{
		((Control)button2).BackColor = Color.White;
	}

	private void Form1_FormClosing(object sender, FormClosingEventArgs e)
	{
		//IL_0005: Unknown result type (might be due to invalid IL or missing references)
		MessageBox.Show(Resources.FunValue);
	}

	protected override void Dispose(bool disposing)
	{
		if (disposing && components != null)
		{
			components.Dispose();
		}
		((Form)this).Dispose(disposing);
	}

	private void InitializeComponent()
	{
		//IL_0011: Unknown result type (might be due to invalid IL or missing references)
		//IL_001b: Expected O, but got Unknown
		//IL_001c: Unknown result type (might be due to invalid IL or missing references)
		//IL_0026: Expected O, but got Unknown
		//IL_0027: Unknown result type (might be due to invalid IL or missing references)
		//IL_0031: Expected O, but got Unknown
		//IL_0032: Unknown result type (might be due to invalid IL or missing references)
		//IL_003c: Expected O, but got Unknown
		//IL_003d: Unknown result type (might be due to invalid IL or missing references)
		//IL_0047: Expected O, but got Unknown
		//IL_0048: Unknown result type (might be due to invalid IL or missing references)
		//IL_0052: Expected O, but got Unknown
		//IL_008c: Unknown result type (might be due to invalid IL or missing references)
		//IL_0096: Expected O, but got Unknown
		//IL_02f2: Unknown result type (might be due to invalid IL or missing references)
		//IL_02fc: Expected O, but got Unknown
		//IL_0458: Unknown result type (might be due to invalid IL or missing references)
		//IL_0462: Expected O, but got Unknown
		//IL_04c3: Unknown result type (might be due to invalid IL or missing references)
		//IL_04cd: Expected O, but got Unknown
		ComponentResourceManager componentResourceManager = new ComponentResourceManager(typeof(capt));
		button1 = new Button();
		label1 = new Label();
		textBox1 = new TextBox();
		textBox2 = new TextBox();
		label2 = new Label();
		button2 = new Button();
		((Control)this).SuspendLayout();
		((ButtonBase)button1).FlatAppearance.BorderSize = 0;
		((ButtonBase)button1).FlatStyle = (FlatStyle)0;
		((Control)button1).Font = new Font("Microsoft Sans Serif", 12f, (FontStyle)0, (GraphicsUnit)3, (byte)204);
		((Control)button1).ForeColor = SystemColors.WindowText;
		((Control)button1).Location = new Point(142, 116);
		((Control)button1).Name = "button1";
		((Control)button1).Size = new Size(249, 47);
		((Control)button1).TabIndex = 0;
		((Control)button1).Text = "Проверить пароль";
		((ButtonBase)button1).UseVisualStyleBackColor = true;
		((Control)button1).BackColorChanged += button1_BackColorChanged;
		((Control)button1).Click += button1_Click;
		((Control)button1).MouseHover += button1_MouseHover;
		((Control)label1).AutoSize = true;
		((Control)label1).Location = new Point(139, 9);
		((Control)label1).Name = "label1";
		((Control)label1).Size = new Size(0, 13);
		((Control)label1).TabIndex = 1;
		((Control)textBox1).Location = new Point(142, 39);
		((TextBoxBase)textBox1).Multiline = true;
		((Control)textBox1).Name = "textBox1";
		((Control)textBox1).Size = new Size(249, 71);
		((Control)textBox1).TabIndex = 2;
		((Control)textBox1).TextChanged += textBox1_TextChanged;
		((Control)textBox2).Location = new Point(142, 183);
		((TextBoxBase)textBox2).Multiline = true;
		((Control)textBox2).Name = "textBox2";
		((Control)textBox2).Size = new Size(249, 68);
		((Control)textBox2).TabIndex = 3;
		((Control)label2).AutoSize = true;
		((Control)label2).Location = new Point(139, 166);
		((Control)label2).Name = "label2";
		((Control)label2).Size = new Size(0, 13);
		((Control)label2).TabIndex = 4;
		((ButtonBase)button2).FlatAppearance.BorderSize = 0;
		((ButtonBase)button2).FlatStyle = (FlatStyle)0;
		((Control)button2).Font = new Font("Microsoft Sans Serif", 12f, (FontStyle)0, (GraphicsUnit)3, (byte)204);
		((Control)button2).Location = new Point(142, 257);
		((Control)button2).Name = "button2";
		((Control)button2).Size = new Size(249, 47);
		((Control)button2).TabIndex = 5;
		((Control)button2).Text = "Проверить секретный ключ";
		((ButtonBase)button2).UseVisualStyleBackColor = true;
		((Control)button2).BackColorChanged += button2_BackColorChanged;
		((Control)button2).Click += button2_Click;
		((Control)button2).MouseHover += button2_MouseHover;
		((ContainerControl)this).AutoScaleDimensions = new SizeF(6f, 13f);
		((ContainerControl)this).AutoScaleMode = (AutoScaleMode)1;
		((Control)this).BackColor = Color.White;
		((Form)this).ClientSize = new Size(549, 312);
		((Control)this).Controls.Add((Control)(object)label1);
		((Control)this).Controls.Add((Control)(object)button2);
		((Control)this).Controls.Add((Control)(object)label2);
		((Control)this).Controls.Add((Control)(object)textBox2);
		((Control)this).Controls.Add((Control)(object)textBox1);
		((Control)this).Controls.Add((Control)(object)button1);
		((Form)this).Icon = (Icon)componentResourceManager.GetObject("$this.Icon");
		((Control)this).MaximumSize = new Size(565, 351);
		((Control)this).MinimumSize = new Size(565, 351);
		((Control)this).Name = "capt";
		((Form)this).Opacity = 0.97;
		((Form)this).ShowIcon = false;
		((Form)this).ShowInTaskbar = true;
		((Form)this).StartPosition = (FormStartPosition)1;
		((Form)this).FormClosing += new FormClosingEventHandler(Form1_FormClosing);
		((Control)this).ResumeLayout(false);
		((Control)this).PerformLayout();
	}
}
