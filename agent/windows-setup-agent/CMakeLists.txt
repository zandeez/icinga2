
if(MSVC)

include(CSharpUtilities)

add_executable(Icinga2SetupAgent
	App.config
	app.manifest
	icinga-banner.png
	icinga.ico

	EndpointInputBox.Designer.cs
	EndpointInputBox.cs
	EndpointInputBox.resx
	GlobalZonesInputBox.Designer.cs
	GlobalZonesInputBox.cs
	GlobalZonesInputBox.resx
	Program.cs
	ServiceStatus.Designer.cs
	ServiceStatus.cs
	ServiceStatus.resx
	SetupWizard.Designer.cs
	SetupWizard.cs
	SetupWizard.resx

	Properties/AssemblyInfo.cs
	Properties/Resources.Designer.cs
	Properties/Resources.resx
	Properties/Settings.Designer.cs
	Properties/Settings.settings
)

csharp_set_designer_cs_properties(
	Properties/AssemblyInfo.cs
	Properties/Resources.Designer.cs
	Properties/Resources.resx
	Properties/Settings.Designer.cs
	Properties/Settings.settings
)

set_property(TARGET Icinga2SetupAgent PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v2.0.0")
set_property(TARGET Icinga2SetupAgent PROPERTY WIN32_EXECUTABLE TRUE)
set_property(TARGET Icinga2SetupAgent PROPERTY VS_DOTNET_REFERENCES
	"Microsoft.CSharp"
	"System"
	"System.Core"
	"System.Collections.Generic"
	"System.ComponentModel"
	"System.Data"
	"System.Drawing"
	"System.Text"
	"System.Windows.Forms"
	"System.ServiceProcess"
	"System.Diagnostics"
	"System.IO"
	"System.Text"
	"System.Runtime.InteropServices"
	"System.Security.Cryptography.X509Certificates"
	"System.Threading"
	"System.Net.NetworkInformation"
	"System.IO.Compression"
	"System.Security.AccessControl"
)

install(
	FILES ${CMAKE_CURRENT_SOURCE_DIR}/windows-setup-agent/bin/\${CMAKE_INSTALL_CONFIG_NAME}/Icinga2SetupAgent.exe ${CMAKE_CURRENT_SOURCE_DIR}/windows-setup-agent/bin/\${CMAKE_INSTALL_CONFIG_NAME}/Icinga2SetupAgent.exe.config
	DESTINATION ${CMAKE_INSTALL_SBINDIR}
)

endif() # MSVC
