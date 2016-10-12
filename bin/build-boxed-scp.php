<?php
/**
 * WPИ-XM Server Stack
 * Copyright © 2010 - 2015 Jens-André Koch <jakoch@web.de>
 * http://wpn-xm.org/
 *
 * This source file is subject to the terms of the MIT license.
 * For full copyright and license information, view the bundled LICENSE file.
 */

$version = getVersion();

repackage($version, 'x86');
repackage($version, 'x86_64');

/**
 * Repackage
 *
 * 1. download the latest version of the server control panel
 * 2. unzip
 * 3. use EngimaVirtualBox helper script to box the Qt application
 *    - build an EVB project from the unzipped application folder
 *    - "box" the Qt application: result is a single executable.
 * 4. zip the executable (the file suffix is "_boxed.zip").
 * 5. cleanup
 */
function repackage($version, $bitsize)
{
    // 1
    download('https://github.com/WPN-XM/server-control-panel/releases/download/v'. $version . '/wpnxm-scp-'. $version . '-'. $bitsize .'.zip',
             'wpnxm-scp-'. $bitsize .'.zip');
    // 2
    unzip('wpnxm-scp-'. $bitsize .'.zip', __DIR__ . '/wpnxm-scp-'. $bitsize);
    // 3
    passthru('php EnigmaVirtualBox.php wpnxm-scp-'. $bitsize .' wpn-xm.exe wpn-xm-'. $bitsize .'_boxed.exe');
    rename(__DIR__ . '\wpn-xm-'. $bitsize .'_boxed.exe', __DIR__ . '\wpn-xm.exe');
    // 4    
    $7zipPath = ($bitsize === 'x86_64') ? '\7zip\x64' : '\7zip\x86';
    passthru(__DIR__ . $7zipPath . '\7za.exe a -tzip wpnxm-scp-'. $version . '-'. $bitsize .'_boxed.zip wpn-xm.exe -mx9 -mmt');
    // 5
    unlink(__DIR__ . '\wpn-xm.exe');
    unlink(__DIR__ . '\wpnxm-scp-'. $bitsize .'.zip');
    unlink(__DIR__ . '\project.evb');
}

function xml2array($xmlString)
{
    $xml = simplexml_load_string($xmlString, 'SimpleXMLElement', LIBXML_NOCDATA);

    return json_decode(json_encode((array)$xml), TRUE);
}

function getVersion()
{
    $buildfile = dirname(__DIR__) . '/build.xml';
    $content = file_get_contents($buildfile);
    $array = xml2array($content);

    $version = $array['property'][0]['@attributes']['value'] . '.' .
               $array['property'][1]['@attributes']['value'] . '.' .
               $array['property'][2]['@attributes']['value'];

    return $version;
}

function unzip($zipfile, $folder)
{
    $zip = new ZipArchive;
    $res = $zip->open($zipfile);
    if ($res === TRUE) {
        $zip->extractTo($folder);
        $zip->close();
    }
}

function download($url, $targetFile)
{
    // disable the SSL certificate check
    $context = stream_context_create([
        "ssl" => [
            "verify_peer" => false,
            "verify_peer_name" => false,
        ]
    ]);

    $file = fopen($url, "rb", false, $context);

    if ($file) {
        $newf = fopen($targetFile, "wb");

        if ($newf) {
            while(!feof($file)) {
                fwrite($newf, fread($file, 1024 * 8), 1024 * 8);
            }
        }
    }

    if ($file) {
        fclose($file);
    }

    if ($newf) {
        fclose($newf);
    }
 }