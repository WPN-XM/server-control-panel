<?php
/**
 * WPИ-XM Server Stack
 * Copyright © 2010 - 2015 Jens-André Koch <jakoch@web.de>
 * http://wpn-xm.org/
 *
 * This source file is subject to the terms of the MIT license.
 * For full copyright and license information, view the bundled LICENSE file.
 */

/**
 * Enigma Virtual Box - Configuration Generation and Boxing Tool
 *
 * This util generates a "evb" packaging configuration file.
 * It includes all files and folders of a given folder with absolute paths.
 * Then it uses "enigmavbconole" to box the application.
 *
 * CLI example:
 * > php evb.php wpnxm-scp-x86 wpn-xm.exe wpn-xm_x86_boxed.exe
 * > php evb.php wpnxm-scp-x86_64 wpn-xm.exe wpn-xm_x86_64_boxed.exe
 */

class EnigmaVirtualBox
{
	public function __construct()
	{
		// pull arguments from global scope and use $argv inside a class
		global $argv;

		#var_dump($argv);

		// fetch command line arguments

		 // the application folder to scan or "--help"
		if(empty($argv[1]) or $argv[1] === '--help' or $argv[1] === '/?') {
			$this->print_cli_header();
			echo "Usage: EngimaVirtualBox.php c:\\folder_to_box input.exe output_boxed.exe\n";
			echo "       EngimaVirtualBox.php c:\\folder_to_box input.exe\n";
			exit;
		} else {
			$folder = $argv[1];
		}

        // the application executable (residing in the folder to scan)
		if(empty($argv[2])) {
			$this->print_cli_header();
			exit("CLI Argument 2 is missing. No input exe given.");
		} else {
			$exe_in = $argv[2];
		}

       // the named of the boxed application executable
		if(empty($argv[3])) {
			$this->print_cli_header();
			exit("CLI Argument 3 is missing. No output exe given.");
		} else {
			$exe_out = $argv[3];
		}

		// arguments logic checks

		if(!is_dir($folder)) {
			$this->print_cli_header();
			exit('CLI Argument 1: The folder "' . $folder . '" was not found.');
		}

		$fullpath_exe_in = realpath($folder) . DIRECTORY_SEPARATOR . $exe_in;
		if(!is_file($fullpath_exe_in)) {
			$this->print_cli_header();
			exit('CLI Argument 2: The input executable "' . $fullpath_exe_in . '" was not found.');
		}

		if($exe_in === $exe_out) {
			$this->print_cli_header();
			exit('CLI Argument 2 and CLI Argument 3 are equal. The executable names must differ!');
		}

		// set properties

		$this->folder = $folder;
		$this->exe_in = $exe_in;
		$this->exe_out = $exe_out;
	}

	function print_cli_header()
	{
			echo "Enigma Virtual Box - Config Generator\n\n";
			echo "Copyright (c) 2010 - " . date('Y') . " Jens A. Koch <jakoch@web.de>\n\n";
	}

	function evb_config_header($exe_in, $exe_out)
	{
		$xml =
			"<?xml encoding=\"utf-8\"?>\n" .
			"<>\n" .
			"    <InputFile>" . $exe_in . "</InputFile>\n" .
			"    <OutputFile>" . $exe_out . "</OutputFile>\n" .
			"    <Files>\n" .
			"        <Enabled>true</Enabled>\n" .
			"        <DeleteExtractedOnExit>true</DeleteExtractedOnExit>\n" .
			"        <CompressFiles>false</CompressFiles>\n";

	    return $xml;
	}

	function evb_config_registries()
	{
		$xml =
			"    </Files>\n";
			"    <Registries>\n" .
		    "        <Enabled>false</Enabled>\n" .
		    "        <Registries>\n" .
		    "            <Registry>\n" .
		    "                <Type>1</Type>\n" .
		    "                <Virtual>true</Virtual>\n" .
		    "                <Name>Classes</Name>\n" .
		    "                <ValueType>0</ValueType>\n" .
		    "                <Value/>\n" .
		    "                <Registries/>\n" .
		    "            </Registry>\n" .
		    "            <Registry>\n" .
		    "                <Type>1</Type>\n" .
		    "                <Virtual>true</Virtual>\n" .
		    "                <Name>User</Name>\n" .
		    "                <ValueType>0</ValueType>\n" .
		    "                <Value/>\n" .
		    "                <Registries/>\n" .
		    "            </Registry>\n" .
		    "            <Registry>\n" .
		    "                <Type>1</Type>\n" .
		    "                <Virtual>true</Virtual>\n" .
		    "                <Name>Machine</Name>\n" .
		    "                <ValueType>0</ValueType>\n" .
		    "                <Value/>\n" .
		    "                <Registries/>\n" .
		    "            </Registry>\n" .
		    "            <Registry>\n" .
		    "                <Type>1</Type>\n" .
		    "                <Virtual>true</Virtual>\n" .
		    "                <Name>Users</Name>\n" .
		    "                <ValueType>0</ValueType>\n" .
		    "                <Value/>\n" .
		    "                <Registries/>\n" .
		    "            </Registry>\n" .
		    "            <Registry>\n" .
		    "                <Type>1</Type>\n" .
		    "                <Virtual>true</Virtual>\n" .
		    "                <Name>Config</Name>\n" .
		    "                <ValueType>0</ValueType>\n" .
		    "                <Value/>\n" .
		    "                <Registries/>\n" .
		    "            </Registry>\n" .
		    "        </Registries>\n" .
		    "    </Registries>\n";

		return $xml;
	}

	function evb_config_footer()
	{
		$xml =
			"    <Packaging>\n" .
		    "        <Enabled>false</Enabled>\n" .
		    "    </Packaging>\n" .
		    "    <Options>\n" .
		    "        <ShareVirtualSystem>false</ShareVirtualSystem>\n" .
		    "        <MapExecutableWithTemporaryFile>false</MapExecutableWithTemporaryFile>\n" .
		    "        <AllowRunningOfVirtualExeFiles>false</AllowRunningOfVirtualExeFiles>\n" .
		    "    </Options>\n" .
		    "</>\n";

		return $xml;
	}

	function getFiles()
	{
		$path = realpath($this->folder);

		$files = array();

		$rdi = new RecursiveDirectoryIterator($path, RecursiveDirectoryIterator::SKIP_DOTS);
		$rii = new RecursiveIteratorIterator($rdi, RecursiveIteratorIterator::CHILD_FIRST);

		foreach($rii as $splFileInfo)
		{
			// exclude exe_in
			if($splFileInfo->getFilename() === $this->exe_in) {
				continue;
			}

		    $path = $splFileInfo->isDir()
		             ? array($splFileInfo->getFilename() => array())
			         : array($splFileInfo->getFilename());

		    for ($depth = $rii->getDepth() - 1; $depth >= 0; $depth--) {
		        $path = array($rii->getSubIterator($depth)->current()->getFilename() => $path);
		    }

   			$files = array_merge_recursive($files, $path);
		}

		return $files;
	}

	function evb_config_files($files)
	{
		// start with open files and default folder
		$xml =
		 	"        <Files>\n" .
		    "            <File>\n" .
		    "                <Type>3</Type>\n" .
		    "                <Name>%DEFAULT FOLDER%</Name>\n" .
		    "                <Files>\n";

		// files in folder template (header)
		$xml_open_file =
		    "            <File>\n" .
		    "                <Type>3</Type>\n" .
		    "                <Name>%DEFAULT FOLDER%</Name>\n" .
		    "                <Files>\n";

		// files in folder template (footer)
		$xml_close_file =
			"                </Files>\n" .
		    "            </File>\n";

		foreach($files as $i => $file)
		{
			// dir with files
			if(is_array($file))
			{
				// add the folder (type 3)
				$xml .= str_replace('%DEFAULT FOLDER%', $i, $xml_open_file);

				// add the files (type 2)
				foreach($file as $idx => $f) {
					$xml .= $this->evb_xml_file($f, realpath($this->folder) . DIRECTORY_SEPARATOR . $i . DIRECTORY_SEPARATOR . $f);
				}

				// close folder
				$xml .= $xml_close_file;
			}

			// a file
			if(is_string($file)) {
				$xml .= $this->evb_xml_file($file, realpath($this->folder) . DIRECTORY_SEPARATOR . $file);
			}
		}

		// close default folder
		$xml .= "                </Files>\n" .
			    "            </File>\n" .
			    "        </Files>\n";

		return $xml;
	}

	function evb_xml_file($file, $path)
	{
		$xml =  "                    <File>\n" .
			    "                        <Type>2</Type>\n" .
			    "                        <Name>" . $file . "</Name>\n" .
			    "                        <File>" . $path . "</File>\n" .
			    "                        <ActiveX>false</ActiveX>\n" .
			    "                        <ActiveXInstall>false</ActiveXInstall>\n" .
			    "                        <Action>0</Action>\n" .
			    "                        <OverwriteDateTime>false</OverwriteDateTime>\n" .
			    "                        <OverwriteAttributes>false</OverwriteAttributes>\n" .
			    "                        <PassCommandLine>false</PassCommandLine>\n" .
			    "                    </File>\n";

		return $xml;
	}

	function build_xml()
	{
		$this->xml = $this->evb_config_header(
			realpath($this->folder) . DIRECTORY_SEPARATOR .$this->exe_in,
			__DIR__ . DIRECTORY_SEPARATOR . $this->exe_out
		);
		$this->xml .= $this->evb_config_files($this->getFiles());
		$this->xml .= $this->evb_config_registries();
		$this->xml .= $this->evb_config_footer();
	}

	function write_evb_config()
	{
		$this->build_xml();

		file_put_contents('project.evb', $this->xml);
	}

	function box()
	{
		$this->write_evb_config();

		passthru(__DIR__ . '\evb\enigmavbconsole.exe project.evb');
	}
}

$evb = new EnigmaVirtualBox;
$evb->box();