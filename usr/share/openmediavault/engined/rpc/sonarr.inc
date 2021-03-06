<?php

/**
 * Copyright (C) 2010-2012 Ian Moore <imooreyahoo@gmail.com>
 * Copyright (C) 2013-2015 OpenMediaVault Plugin Developers.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 //

namespace OMV\Engined\Rpc;

use DateTime;
use Exception;
use OMV\Config\Database;
use OMV\Engine\Notify\Dispatcher;
use OMV\Rpc\ServiceAbstract;
use OMV\Rpc\ServiceManager as RpcServiceManager;
use OMV\System\Process;
use SplFileObject;

class Sonarr extends ServiceAbstract
{
    private $SonarrSettingsFileA = "/home/sonarr/.config/NzbDrone/config.xml";
    private $SonarrSettingsFileB = "/home/sonarr/.config/Sonarr/config.xml";
    private $database;

    private $dataModelPath = 'conf.service.sonarr';

    private $dispatcher;

    private $eventMessagePath = 'org.openmediavault.conf.services.sonarr';

    public function __construct(Database $database = null, Dispatcher $dispatcher = null)
    {
        $this->database = $database ?: Database::getInstance();
        $this->dispatcher = $dispatcher ?: Dispatcher::getInstance();
    }

    public function getName()
    {
        return 'Sonarr';
    }

    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
    }

    public function getSettings($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);

        $object = $this->database->get($this->dataModelPath);

        $port = 8989;

        // Get current port as set in Sonarr's config file
        if (file_exists($this->SonarrSettingsFileA)) {
            $config = simplexml_load_file(
                $this->SonarrSettingsFileA
            );

            if (!empty($config->Port)) {
                $port = $config->Port;
            }
        }elseif (file_exists($this->SonarrSettingsFileB)) {
            $config = simplexml_load_file(
                $this->SonarrSettingsFileB
            );

            if (!empty($config->Port)) {
                $port = $config->Port;
            }
        }

        $object->add('port', 'integer');
        $object->set('port', intval($port));

        return $object->getAssoc();
    }

    public function setSettings($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.sonarr.setsettings');

        $object = $this->database->get($this->dataModelPath);
        $object->setAssoc($params);

        $this->database->set($object);
        $this->dispatcher->notify(OMV_NOTIFY_MODIFY, $this->eventMessagePath, $object);

        return $object->getAssoc();
    }
}
