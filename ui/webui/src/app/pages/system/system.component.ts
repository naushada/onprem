import { Component } from '@angular/core';
import { HttpService } from 'src/common/http.service';

import {ClrDatagridStringFilterInterface} from "@clr/angular";

class SerialnumberFilter implements ClrDatagridStringFilterInterface<string> {
    accepts(serialnumber: string, search: string):boolean {
        return serialnumber == search;
    }
}

class DeviceStatusFilter implements ClrDatagridStringFilterInterface<string> {
  accepts(devicestatus: string, search: string):boolean {
      return devicestatus == search;
  }
}

class ProductFilter implements ClrDatagridStringFilterInterface<string> {
  accepts(productname: string, search: string):boolean {
      return productname == search;
  }
}


@Component({
  selector: 'app-system',
  templateUrl: './system.component.html',
  styleUrls: ['./system.component.scss']
})

export class SystemComponent {

  selectedDevice:any;
  devices: any[] = [];
  
  //Custom Filter
  serialnumberFilter = new SerialnumberFilter();
  devicestatusFilter = new DeviceStatusFilter();
  productFilter = new ProductFilter();

  constructor(private http: HttpService) {
    http.getdevicedetails().subscribe((response: Array<string>) => {
      this.devices.length = 0;
      response.forEach(ent => {this.devices.push(ent);})
    },
    (error) => {},
    () => {});
  }

  onSelectionChanged(event:any) {
  }
}
