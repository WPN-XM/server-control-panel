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
$bitsize = (isset($_ENV['bitsize']) && $_ENV['bitsize'] == 'x64') ? 'x86_64' : 'x86';

package($version, $bitsize);

/**
 * Package
 *
 * 1. use EngimaVirtualBox helper script to box the Qt application
 *    - build an EVB project from the unzipped application folder
 *    - "box" the Qt application: result is a single executable.
 * 2. rename "wpn-xm_boxed.exe" to "wpn-xm.exe"
 * 3. zip the executable (the file suffix is "_boxed.zip").
 * 4. cleanup
 */
function package($version, $bitsize)
{
    // 1 build enigma box
    passthru('php EnigmaVirtualBox.php ..\release wpn-xm.exe wpn-xm_boxed.exe');
    // 2 overwrite old exe by renaming
    rename(__DIR__ . '\wpn-xm_boxed.exe', __DIR__ . '\wpn-xm.exe');
    // 3 package
    passthru('7z a -tzip wpnxm-scp-v'. $version . '-'. $bitsize .'_boxed.zip wpn-xm.exe -mx9 -mmt');
    // 4 cleanup
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
