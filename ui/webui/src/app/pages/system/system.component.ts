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

  selected:Array<any> = [];
  devices: any[] = [];
  
  //Custom Filter
  serialnumberFilter = new SerialnumberFilter();
  devicestatusFilter = new DeviceStatusFilter();
  productFilter = new ProductFilter();

  constructor(private http: HttpService) {
    http.getdevicedetails().subscribe((response: string) => {
      this.devices.length = 0;
      let result = JSON.parse(JSON.stringify(response));
      //console.log(result);
      if(result["status"] == "success") {
        this.devices = JSON.parse(result["response"]);
        //console.log(this.devices);
      }
      //response.forEach(ent => {this.devices.push(ent);})
    },
    (error) => {},
    () => {});
  }

  onSelectionChanged(event:any) {
  }

  onOpenGWUi(event:any) {
    //console.log(this.http.getEmbeddedUri());
    let uri: string = this.http.getEmbeddedUri() + "/index.html?serialnumber=" + 
        this.selected[0].serialnumber + "&model=" + 
        this.selected[0].model + "&version=" + this.selected[0].osversion;

    window.open(uri);
    
  }

}
